#pragma once

#include "INode.hpp"
#include "Tasks.hpp"
#include "traits/node.hpp"

namespace nil::gate
{
    class Core;
}

namespace nil::gate::detail
{
    template <typename T, typename = void>
    class Node final: public INode
    {
        using input_t = typename detail::traits::node<T>::inputs;
        using sync_output_t = typename detail::traits::node<T>::sync_outputs;
        using async_output_t = typename detail::traits::node<T>::async_outputs;
        using output_t = typename detail::traits::node<T>::outputs;

    public:
        template <typename... Args>
        Node(
            Tasks* init_tasks,
            const typename input_t::edges& init_inputs,
            typename async_output_t::tuple init_asyncs,
            Args&&... args
        )
            : Node(
                  init_tasks,
                  init_inputs,
                  std::move(init_asyncs),
                  typename input_t::type(),
                  typename input_t::make_index_sequence(),
                  typename sync_output_t::make_index_sequence(),
                  typename async_output_t::make_index_sequence(),
                  std::forward<Args>(args)...
              )
        {
        }

        void exec(Core* core) override
        {
            if (state == EState::Pending)
            {
                using return_t = decltype(  //
                    std::declval<Node<T>>() //
                        .call(core, typename input_t::make_index_sequence())
                );
                if constexpr (std::is_same_v<void, return_t>)
                {
                    call(core, typename input_t::make_index_sequence());
                    state = EState::Done;
                }
                else
                {
                    auto result = call(core, typename input_t::make_index_sequence());
                    state = EState::Done;
                    forward_to_output(result, typename sync_output_t::make_index_sequence());
                }
            }
        }

        void pend() override
        {
            if (state != EState::Pending)
            {
                state = EState::Pending;
                pend(typename sync_output_t::make_index_sequence());
            }
        }

        std::uint64_t depth() const override
        {
            return cached_depth;
        }

        typename output_t::edges output_edges()
        {
            return output_edges(
                typename sync_output_t::make_index_sequence(),
                typename async_output_t::make_index_sequence()
            );
        }

    private:
        template <
            typename... I,
            std::size_t... i_indices,
            std::size_t... s_indices,
            std::size_t... a_indices,
            typename... Args>
        Node(
            Tasks* init_tasks,
            const typename input_t::edges& init_inputs,
            [[maybe_unused]] typename async_output_t::tuple init_asyncs,
            traits::types<I...>,
            std::index_sequence<i_indices...>,
            std::index_sequence<s_indices...>,
            std::index_sequence<a_indices...>,
            Args&&... args
        )
            : tasks(init_tasks)
            , state(EState::Pending)
            , instance{std::forward<Args>(args)...}
            , inputs(init_inputs)
            , sync_outputs()
            , async_outputs()
        {
            (..., initialize_input(static_cast<DataEdge<I>*>(get<i_indices>(inputs))));

            (get<s_indices>(sync_outputs).set_depth(cached_depth), ...);
            (get<a_indices>(async_outputs).set_depth(0u), ...);

            (get<s_indices>(sync_outputs).attach_tasks(tasks), ...);
            (get<a_indices>(async_outputs).attach_tasks(tasks), ...);

            (get<a_indices>(async_outputs).exec(std::move(get<a_indices>(init_asyncs))), ...);
        }

        template <std::size_t... s_indices>
        void pend(std::index_sequence<s_indices...>)
        {
            (get<s_indices>(sync_outputs).pend(), ...);
        }

        template <std::size_t... i_indices>
        auto call(Core* core, std::index_sequence<i_indices...>)
        {
            if constexpr (traits::node<T>::has_async)
            {
                if constexpr (traits::node<T>::has_core)
                {
                    return instance(
                        async_edges(typename async_output_t::make_index_sequence()),
                        *core,
                        get<i_indices>(inputs)->value()...
                    );
                }
                else
                {
                    return instance(
                        async_edges(typename async_output_t::make_index_sequence()),
                        get<i_indices>(inputs)->value()...
                    );
                }
            }
            else
            {
                return instance(get<i_indices>(inputs)->value()...);
            }
        }

        template <std::size_t... s_indices>
        auto forward_to_output(sync_output_t::tuple& result, std::index_sequence<s_indices...>)
        {
            (get<s_indices>(sync_outputs).exec(std::move(get<s_indices>(result))), ...);
        }

        template <std::size_t... a_indices>
        auto async_edges(std::index_sequence<a_indices...>)
        {
            return typename async_output_t::edges( //
                std::addressof(get<a_indices>(async_outputs))...
            );
        }

        template <std::size_t... s_indices, std::size_t... a_indices>
        auto output_edges(std::index_sequence<s_indices...>, std::index_sequence<a_indices...>)
        {
            return typename output_t::edges(
                std::addressof(get<s_indices>(sync_outputs))...,
                std::addressof(get<a_indices>(async_outputs))...
            );
        }

        template <typename U>
        void initialize_input(DataEdge<U>* edge)
        {
            edge->attach_output(this);
            cached_depth = std::max(cached_depth, edge->depth());
        }

        Tasks* tasks;
        EState state;

        T instance;
        typename input_t::edges inputs;
        typename sync_output_t::data_edges sync_outputs;
        typename async_output_t::data_edges async_outputs;

        std::uint64_t cached_depth = 0u;
    };
}
