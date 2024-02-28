#pragma once

#include "detail/Node.hpp"
#include "detail/edge/Batch.hpp"
#include "detail/edge/Data.hpp"

namespace nil::gate
{
    /**
     * @brief Owns the whole graph (Nodes/Edges)
     *  -  Nodes represent a runnable object
     *  -  Edges owns the data
     *  -  Edge type qualifications:
     *      -  not pointer
     *      -  not reference
     *      -  if std::unique_ptr/std::shared_ptr, T should be const
     *  -  Node type qualifications:
     *      -  not pointer
     *      -  if reference, should be const
     *      -  copy-able
     *      -  if std::unique_ptr/std::shared_ptr, T should be const
     */
    class Core final
    {
    public:
        Core() = default;
        ~Core() noexcept = default;

        Core(Core&&) = delete;
        Core(const Core&) = delete;
        Core& operator=(Core&&) = delete;
        Core& operator=(const Core&) = delete;

        // Invalid types for input/output detected
        template <typename T, typename... Args>
        std::enable_if_t<
            !detail::callable_traits<T>::is_valid,
            typename detail::callable_traits<T>::outputs::readonly_edges> //
            node(typename detail::callable_traits<T>::inputs::readonly_edges edges, Args&&... args)
            = delete;

        /**
         * @brief create a node
         *
         * @tparam T                                node type
         * @tparam Args
         * @param edges                             ReadOnlyEdge/MutableEdge pointers
         * @param args                              additional constructor arguments
         * @return `std::tuple<ReadOnlyEdge*, ...>` output edges (still owned by core)
         */
        template <typename T, typename... Args>
        std::enable_if_t<
            detail::callable_traits<T>::is_valid && !detail::callable_traits<T>::has_async,
            typename detail::callable_traits<T>::outputs::readonly_edges>
            node(
                typename detail::callable_traits<T>::inputs::readonly_edges input_edges,
                Args&&... args
            )
        {
            // TODO: validate if input_edges is owned by this core.
            auto node = std::make_unique<detail::Node<T>>(
                &tasks,
                input_edges,
                std::tuple<>(),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(node)).get())
                ->output_edges();
        }

        /**
         * @brief create a node
         *
         * @tparam T                                node type
         * @tparam Args
         * @param edges                             ReadOnlyEdge/MutableEdge pointers
         * @param args                              additional constructor arguments
         * @return `std::tuple<ReadOnlyEdge*, ...>` output edges (still owned by core)
         */
        template <typename T, typename... Args>
        std::enable_if_t<
            detail::callable_traits<T>::is_valid && detail::callable_traits<T>::has_async,
            typename detail::callable_traits<T>::outputs::readonly_edges>
            node(
                typename detail::callable_traits<T>::async_outputs::tuple async_initilizer,
                typename detail::callable_traits<T>::inputs::readonly_edges input_edges,
                Args&&... args
            )
        {
            // TODO: validate if input_edges is owned by this core.
            auto node = std::make_unique<detail::Node<T>>(
                &tasks,
                input_edges,
                std::move(async_initilizer),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(node)).get())
                ->output_edges();
        }

        // Invalid type passed
        template <typename T>
        std::enable_if_t<!detail::edge_validate<T>::value, MutableEdge<T>*> edge() = delete;

        /**
         * @brief create an edge
         *
         * @tparam T                    - edge type
         * @return `MutableEdge<T>*`    - edge instance (still owned by core)
         */
        template <typename T, typename... Args>
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge(Args&&... args)
        {
            return static_cast<MutableEdge<T>*>(
                required_edges
                    .emplace_back(
                        std::make_unique<detail::DataEdge<T>>(&tasks, std::forward<Args>(args)...)
                    )
                    .get()
            );
        }

        /**
         * @brief create an edge
         *
         * @tparam T                    - edge type
         * @return `MutableEdge<T>*`    - edge instance (still owned by core)
         */
        template <typename T>
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge(T&& value)
        {
            return static_cast<MutableEdge<T>*>(
                required_edges
                    .emplace_back(
                        std::make_unique<detail::DataEdge<T>>(&tasks, std::forward<T>(value))
                    )
                    .get()
            );
        }

        /**
         * @brief run all pending nodes
         */
        void run()
        {
            for (const auto& d : tasks.flush())
            {
                d->call();
            }

            for (const auto& node : owned_nodes)
            {
                node->exec(commit_cb.get());
            }
        }

        template <typename... T>
        detail::Batch<T...> batch(MutableEdge<T>*... edges)
        {
            return detail::Batch<T...>(
                &tasks,
                commit_cb.get(),
                {static_cast<detail::InternalEdge<T>*>(edges)...}
            );
        }

        void commit()
        {
            if (commit_cb)
            {
                commit_cb->call();
            }
        }

        template <typename CB>
        void set_commit(CB cb)
        {
            struct Callable: detail::ICallable
            {
                Callable(Core& init_self, CB init_callback)
                    : self(init_self)
                    , callback(std::move(init_callback))
                {
                }

                void call() override
                {
                    callback(self);
                }

                Core& self;
                CB callback;
            };

            commit_cb = std::make_unique<Callable>(*this, std::move(cb));
        }

    private:
        detail::Tasks tasks;
        std::unique_ptr<detail::ICallable> commit_cb;
        std::vector<std::unique_ptr<detail::INode>> owned_nodes;
        std::vector<std::unique_ptr<IEdge>> required_edges;
    };
}
