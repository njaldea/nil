#pragma once

#include "Batch.hpp"
#include "detail/Node.hpp"
#include "edges/Mutable.hpp"
#include "errors.hpp"
#include "traits/edgify.hpp"

#include "detail/traits/node.hpp"

namespace nil::gate::concepts
{
    template <typename T>
    concept is_node_invalid = !detail::traits::node<T>::is_valid;

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
    namespace errors
    {
        template <typename T>
        struct Errors
        {
            using traits = nil::gate::detail::traits::node<T>;
            Error inputs = Check<traits::inputs::is_valid>();
            Error sync_outputs = Check<traits::sync_outputs::is_valid>();
            Error async_outputs = Check<traits::async_outputs::is_valid>();
            Error asyncs = Check<traits::input_resolver_t::is_async_valid>();
            Error core = Check<traits::input_resolver_t::is_core_valid>();
        };
    }

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

        template <concepts::is_node_invalid T>
        outputs_t<T> node(
            T instance,
            errors::Errors<T> = errors::Errors<T>() //
        );
        template <concepts::is_node_invalid T>
        outputs_t<T> node(
            T instance,
            inputs_t<T> edges,
            errors::Errors<T> = errors::Errors<T>() //
        );
        template <concepts::is_node_invalid T>
        outputs_t<T> node(
            T instance,
            asyncs_t<T> async_init,
            inputs_t<T> edges,
            errors::Errors<T> = errors::Errors<T>()
        );
        template <concepts::is_node_invalid T>
        outputs_t<T> node(
            T instance,
            asyncs_t<T> async_init,
            errors::Errors<T> = errors::Errors<T>() //
        );

        template <concepts::has_input_no_async T>
        outputs_t<T> node(T instance, inputs_t<T> input_edges)
        {
            auto n = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                asyncs_t<T>(),
                std::move(instance)
            );
            auto r = owned_nodes.emplace_back(std::move(n)).get();
            return static_cast<detail::Node<T>*>(r)->output_edges();
        }

        template <concepts::no_input_no_async T>
        outputs_t<T> node(T instance)
        {
            auto n = std::make_unique<detail::Node<T>>(
                tasks.get(),
                inputs_t<T>(),
                asyncs_t<T>(),
                std::move(instance)
            );
            auto r = owned_nodes.emplace_back(std::move(n)).get();
            return static_cast<detail::Node<T>*>(r)->output_edges();
        }

        template <concepts::has_input_has_async T>
        outputs_t<T> node(T instance, asyncs_t<T> async_init, inputs_t<T> input_edges)
        {
            auto n = std::make_unique<detail::Node<T>>(
                tasks.get(),
                input_edges,
                std::move(async_init),
                std::move(instance)
            );
            auto r = owned_nodes.emplace_back(std::move(n)).get();
            return static_cast<detail::Node<T>*>(r)->output_edges();
        }

        template <concepts::no_input_has_async T>
        outputs_t<T> node(T instance, asyncs_t<T> async_init)
        {
            auto n = std::make_unique<detail::Node<T>>(
                tasks.get(),
                inputs_t<T>(),
                std::move(async_init),
                std::move(instance)
            );
            auto r = owned_nodes.emplace_back(std::move(n)).get();
            return static_cast<detail::Node<T>*>(r)->output_edges();
        }

        /// starting from this point - edge

        template <typename T>
        edges::Mutable<traits::edgify_t<std::decay_t<T>>>* edge(T value)
        {
            auto e = std::make_unique<detail::edges::Data<traits::edgify_t<std::decay_t<T>>>>(
                tasks.get(),
                std::move(value)
            );
            auto r = required_edges.emplace_back(std::move(e)).get();
            return static_cast<edges::Mutable<traits::edgify_t<std::decay_t<T>>>*>(r);
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
        Batch<T...> batch(edges::Mutable<T>*... edges) const
        {
#ifdef NIL_GATE_CHECKS
            // [TODO] validate if the edges has the same tasks object
#endif
            return Batch<T...>(this, tasks.get(), commit_cb.get(), {edges...});
        }

        template <typename... T>
        Batch<T...> batch(std::tuple<edges::Mutable<T>*...> edges) const
        {
#ifdef NIL_GATE_CHECKS
            // [TODO] validate if the edges has the same tasks object
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
