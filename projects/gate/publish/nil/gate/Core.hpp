#pragma once

#include "Batch.hpp"
#include "MEdge.hpp"
#include "detail/DataEdge.hpp"
#include "detail/Node.hpp"

#include "detail/traits/node.hpp"
#include "detail/validation/edge.hpp"

namespace nil::gate
{
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

        /// starting from this point - node

        // output_edges node<T>(T&&);
        // output_edges node<T>(Args&&...);
        // output_edges node<T>(async_init, T&&);
        // output_edges node<T>(async_init, Args&&...);

        // output_edges node<T>(input_edges, T&&);
        // output_edges node<T>(input_edges, Args&&...);
        // output_edges node<T>(async_init, input_edges, T&&);
        // output_edges node<T>(async_init, input_edges, Args&&...);

        // Invalid types for input/output detected
        template <typename T, typename... Args>
        std::enable_if_t<
            !node_traits<T>::is_valid,
            typename node_traits<T>::outputs::edges> //
            node(typename node_traits<T>::inputs::edges edges, const Args&... /* unused */) noexcept
            = delete;

        template <typename T, typename... Args>
        std::enable_if_t<
            node_traits<T>::is_valid          //
                && !node_traits<T>::has_async //
                && (node_traits<T>::inputs::size > 0),
            typename node_traits<T>::outputs::edges>
            node(typename node_traits<T>::inputs::edges input_edges, Args&&... args)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                std::tuple<>(),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T, typename... Args>
        std::enable_if_t<
            node_traits<T>::is_valid          //
                && !node_traits<T>::has_async //
                && (node_traits<T>::inputs::size > 0),
            typename node_traits<T>::outputs::edges>
            node(typename node_traits<T>::inputs::edges input_edges, T&& node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                std::tuple<>(),
                std::forward<T>(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T, typename... Args>
        std::enable_if_t<
            node_traits<T>::is_valid          //
                && !node_traits<T>::has_async //
                && (node_traits<T>::inputs::size == 0),
            typename node_traits<T>::outputs::edges>
            node(Args&&... args)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                typename node_traits<T>::inputs::edges(),
                std::tuple<>(),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T>
        std::enable_if_t<
            node_traits<T>::is_valid          //
                && !node_traits<T>::has_async //
                && (node_traits<T>::inputs::size == 0),
            typename node_traits<T>::outputs::edges>
            node(T&& node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                typename node_traits<T>::inputs::edges(),
                std::tuple<>(),
                std::forward<T>(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T, typename... Args>
        std::enable_if_t<
            node_traits<T>::is_valid         //
                && node_traits<T>::has_async //
                && (node_traits<T>::inputs::size > 0),
            typename node_traits<T>::outputs::edges>
            node(
                typename node_traits<T>::async_outputs::tuple async_initilizer,
                typename node_traits<T>::inputs::edges input_edges,
                Args&&... args
            )
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                std::move(async_initilizer),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T>
        std::enable_if_t<
            node_traits<T>::is_valid         //
                && node_traits<T>::has_async //
                && (node_traits<T>::inputs::size > 0),
            typename node_traits<T>::outputs::edges>
            node(
                typename node_traits<T>::async_outputs::tuple async_initilizer,
                typename node_traits<T>::inputs::edges input_edges,
                T&& node
            )
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                std::move(async_initilizer),
                std::forward<T>(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T, typename... Args>
        std::enable_if_t<
            node_traits<T>::is_valid         //
                && node_traits<T>::has_async //
                && (node_traits<T>::inputs::size == 0),
            typename node_traits<T>::outputs::edges>
            node(typename node_traits<T>::async_outputs::tuple async_initilizer, Args&&... args)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                typename node_traits<T>::inputs::edges(),
                std::move(async_initilizer),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <typename T>
        std::enable_if_t<
            node_traits<T>::is_valid         //
                && node_traits<T>::has_async //
                && (node_traits<T>::inputs::size == 0),
            typename node_traits<T>::outputs::edges>
            node(typename node_traits<T>::async_outputs::tuple async_initilizer, T&& node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                typename node_traits<T>::inputs::edges(),
                std::move(async_initilizer),
                std::forward<T>(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        /// starting from this point - edge

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
        void run() const
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
            return Batch<T...>(this, tasks.get(), commit_cb.get(), {edges...});
        }

        template <typename... T>
        Batch<T...> batch(std::tuple<MutableEdge<T>*...> edges) const
        {
#ifdef NIL_GATE_CHECKS
            // TODO: edges should be in required_edges.
            // by doing this i should be able to simplify BatchEdge
            // and make sure that all edges received by Batch
            // is a DataEdge.
#endif
            return Batch<T...>(this, tasks.get(), commit_cb.get(), edges);
        }

        void commit() const
        {
            if (commit_cb)
            {
                commit_cb->call(this);
            }
        }

        template <typename CB>
        void set_commit(CB cb) noexcept
        {
            struct Callable: detail::ICallable<void(const Core*)>
            {
                explicit Callable(CB&& init_cb)
                    : cb(std::move(init_cb))
                {
                }

                void call(const Core* core) override
                {
                    cb(*core);
                }

                CB cb;
            };

            commit_cb = std::make_unique<Callable>(std::move(cb));
        }

    private:
        std::unique_ptr<detail::Tasks> tasks = std::make_unique<detail::Tasks>();
        std::unique_ptr<detail::ICallable<void(const Core*)>> commit_cb;
        std::vector<std::unique_ptr<detail::INode>> owned_nodes;
        std::vector<std::unique_ptr<IEdge>> required_edges;
    };
}
