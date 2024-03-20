#pragma once

#include "Batch.hpp"
#include "MEdge.hpp"
#include "detail/Node.hpp"

#include "detail/traits/node.hpp"
#include "detail/validation/edge.hpp"

namespace nil::gate::concepts
{
    template <typename T>
    concept is_node_invalid = !detail::traits::node<T>::is_valid;
    template <typename T>
    concept is_edge_invalid = !detail::edge_validate<T>::value;
    template <typename T>
    concept is_edge_valid = detail::edge_validate<T>::value;

    template <typename T>
    concept has_input_has_async                      //
        = detail::traits::node<T>::is_valid          //
        && detail::traits::node<T>::inputs::size > 0 //
        && detail::traits::node<T>::has_async;
    template <typename T>
    concept has_input_no_async                       //
        = detail::traits::node<T>::is_valid          //
        && detail::traits::node<T>::inputs::size > 0 //
        && !detail::traits::node<T>::has_async;
    template <typename T>
    concept no_input_has_async                        //
        = detail::traits::node<T>::is_valid           //
        && detail::traits::node<T>::inputs::size == 0 //
        && detail::traits::node<T>::has_async;
    template <typename T>
    concept no_input_no_async                         //
        = detail::traits::node<T>::is_valid           //
        && detail::traits::node<T>::inputs::size == 0 //
        && !detail::traits::node<T>::has_async;
}

namespace nil::gate
{
    class Core final
    {
        template <typename T>
        using inputs_t = detail::traits::node<T>::inputs::edges;
        template <typename T>
        using outputs_t = detail::traits::node<T>::outputs::edges;
        template <typename T>
        using asyncs_t = detail::traits::node<T>::async_outputs::tuple;

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

        template <concepts::is_node_invalid T, typename... Args>
        outputs_t<T> node(inputs_t<T> edges, const Args&... args) = delete;
        template <concepts::is_node_invalid T>
        outputs_t<T> node(inputs_t<T> edges, T args) = delete;

        template <concepts::has_input_no_async T, typename... Args>
        outputs_t<T> node(inputs_t<T> input_edges, Args&&... args)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                asyncs_t<T>(),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <concepts::has_input_no_async T>
        outputs_t<T> node(inputs_t<T> input_edges, T node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                asyncs_t<T>(),
                std::move(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <concepts::no_input_no_async T, typename... Args>
        outputs_t<T> node(Args&&... args)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                inputs_t<T>(),
                asyncs_t<T>(),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <concepts::no_input_no_async T>
        outputs_t<T> node(T node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                inputs_t<T>(),
                asyncs_t<T>(),
                std::move(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <concepts::has_input_has_async T, typename... Args>
        outputs_t<T> node(asyncs_t<T> async_initilizer, inputs_t<T> input_edges, Args&&... args)
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

        template <concepts::has_input_has_async T>
        outputs_t<T> node(asyncs_t<T> async_initilizer, inputs_t<T> input_edges, T node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                std::move(async_initilizer),
                std::move(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <concepts::no_input_has_async T, typename... Args>
        outputs_t<T> node(asyncs_t<T> async_initilizer, Args&&... args)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                inputs_t<T>(),
                std::move(async_initilizer),
                std::forward<Args>(args)...
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        template <concepts::no_input_has_async T>
        outputs_t<T> node(asyncs_t<T> async_initilizer, T node)
        {
            auto result = std::make_unique<detail::Node<T>>(
                tasks.get(),
                inputs_t<T>(),
                std::move(async_initilizer),
                std::move(node)
            );
            return static_cast<detail::Node<T>*>(owned_nodes.emplace_back(std::move(result)).get())
                ->output_edges();
        }

        /// starting from this point - edge

        template <concepts::is_edge_invalid T>
        MutableEdge<T>* edge() = delete;

        template <concepts::is_edge_valid T>
        MutableEdge<T>* edge()
        {
            return static_cast<MutableEdge<T>*>(
                required_edges.emplace_back(std::make_unique<detail::DataEdge<T>>(tasks.get(), T()))
                    .get()
            );
        }

        template <concepts::is_edge_valid T, typename... Args>
        MutableEdge<T>* edge(Args&&... args)
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

        template <typename T>
        MutableEdge<T>* edge(T value)
        {
            return static_cast<MutableEdge<std::decay_t<T>>*>(
                required_edges
                    .emplace_back(std::make_unique<detail::DataEdge<std::decay_t<T>>>(
                        tasks.get(),
                        std::move(value)
                    ))
                    .get()
            );
        }

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
