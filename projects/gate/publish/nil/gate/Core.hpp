#pragma once

#include "Batch.hpp"
#include "detail/DataEdge.hpp"
#include "detail/Node.hpp"

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
        template <typename T>
        using node_traits = detail::traits::node<T>;

    public:
        Core() = default;
        ~Core() noexcept = default;

        Core(Core&&) = default;
        Core(const Core&) = delete;
        Core& operator=(Core&&) = default;
        Core& operator=(const Core&) = delete;

        // Invalid types for input/output detected
        template <typename T, typename... Args>
        std::enable_if_t<
            !node_traits<T>::is_valid,
            typename node_traits<T>::outputs::edges> //
            node(typename node_traits<T>::inputs::edges edges, Args&&... args) noexcept = delete;

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
            node_traits<T>::is_valid && !node_traits<T>::has_async,
            typename node_traits<T>::outputs::edges>
            node(typename node_traits<T>::inputs::edges input_edges, Args&&... args)
        {
            // TODO: validate if input_edges is owned by this core.
            auto node = std::make_unique<detail::Node<T>>(
                tasks.get(),
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
            node_traits<T>::is_valid && node_traits<T>::has_async,
            typename node_traits<T>::outputs::edges>
            node(
                typename node_traits<T>::async_outputs::tuple async_initilizer,
                typename node_traits<T>::inputs::edges input_edges,
                Args&&... args
            )
        {
            // TODO: validate if input_edges is owned by this core.
            auto node = std::make_unique<detail::Node<T>>(
                tasks.get(),
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
        std::enable_if_t<detail::edge_validate<T>::value, MutableEdge<T>*> edge( //
            Args&&... args
        )
        {
            return static_cast<MutableEdge<T>*>(
                required_edges
                    .emplace_back(std::make_unique<detail::DataEdge<T>>(
                        tasks.get(),
                        std::forward<Args>(args)...
                    ))
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
        MutableEdge<std::decay_t<T>>* edge(T&& value)
        {
            return static_cast<MutableEdge<std::decay_t<T>>*>(
                required_edges
                    .emplace_back(std::make_unique<detail::DataEdge<std::decay_t<T>>>(
                        tasks.get(),
                        std::forward<T>(value)
                    ))
                    .get()
            );
        }

        /**
         * @brief run all pending nodes
         */
        void run()
        {
#ifdef NIL_GATE_CHECKS
            if (!tasks)
            {
                return;
            }
#endif
            for (const auto& d : tasks->flush())
            {
                d->call();
            }

            for (const auto& node : owned_nodes)
            {
                node->exec(this);
            }
        }

        template <typename... T>
        Batch<T...> batch(MutableEdge<T>*... edges) const
        {
#ifdef NIL_GATE_CHECKS
            // TODO: edges should be in required_edges.
            // by doing this i should be able to simplify BatchEdge
            // and make sure that all edges received by Batch
            // is a DataEdge.
#endif
            return Batch<T...>(
                tasks.get(),
                commit_cb.get(),
                {static_cast<detail::DataEdge<T>*>(edges)...}
            );
        }

        void commit() const
        {
            if (commit_cb)
            {
                commit_cb->call();
            }
        }

        template <typename CB>
        void set_commit(CB cb) noexcept
        {
            commit_cb = detail::make_callable([this, cb = std::move(cb)]() { cb(*this); });
        }

    private:
        std::unique_ptr<detail::Tasks> tasks = std::make_unique<detail::Tasks>();
        std::unique_ptr<detail::ICallable> commit_cb;
        std::vector<std::unique_ptr<detail::INode>> owned_nodes;
        std::vector<std::unique_ptr<IEdge>> required_edges;
    };
}
