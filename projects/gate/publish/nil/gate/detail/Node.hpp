#pragma once

#include "INode.hpp"
#include "Tasks.hpp"
#include "types.hpp"

namespace nil::gate::detail
{
    template <typename T, typename = void>
    class Node final: public INode
    {
        using input_t = typename detail::traits<T>::inputs;
        using sync_output_t = typename detail::traits<T>::sync_outputs;
        using async_output_t = typename detail::traits<T>::async_outputs;
        using output_t = typename detail::traits<T>::all_outputs;

    public:
        template <typename... Args>
        Node(
            Tasks* init_tasks,
            ICallable* init_commit,
            const typename input_t::readonly_edges& init_inputs,
            typename async_output_t::tuple init_asyncs,
            Args&&... args
        )
            : Node(
                  init_tasks,
                  init_commit,
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

        void exec() override
        {
            if (state == EState::Pending)
            {
                exec(
                    typename input_t::make_index_sequence(),
                    typename sync_output_t::make_index_sequence(),
                    typename async_output_t::make_index_sequence()
                );
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

        typename output_t::readonly_edges output_edges()
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
            ICallable* init_commit,
            const typename input_t::readonly_edges& init_inputs,
            [[maybe_unused]] typename async_output_t::tuple init_asyncs,
            nil::utils::traits::types<I...>,
            std::index_sequence<i_indices...>,
            std::index_sequence<s_indices...>,
            std::index_sequence<a_indices...>,
            Args&&... args
        )
            : tasks(init_tasks)
            , commit(init_commit)
            , state(EState::Pending)
            , instance{std::forward<Args>(args)...}
            , inputs(init_inputs)
            , sync_outputs()
            , async_outputs()
        {
            (static_cast<InternalEdge<I>*>(get<i_indices>(inputs))->attach_output(this), ...);
            (get<s_indices>(sync_outputs).attach_input(this), ...);
            (get<s_indices>(sync_outputs).attach_tasks(tasks), ...);
            (get<a_indices>(async_outputs).attach_input(this), ...);
            (get<a_indices>(async_outputs).attach_tasks(tasks), ...);
            (get<a_indices>(async_outputs).exec(std::move(get<a_indices>(init_asyncs))), ...);
        }

        template <std::size_t... s_indices>
        void pend(std::index_sequence<s_indices...>)
        {
            (get<s_indices>(sync_outputs).pend(), ...);
        }

        template <std::size_t... i_indices, std::size_t... s_indices, std::size_t... a_indices>
        auto exec(
            std::index_sequence<i_indices...>,
            std::index_sequence<s_indices...>,
            std::index_sequence<a_indices...> //
        )
        {
            constexpr auto has_output = sizeof...(s_indices) > 0;
            constexpr auto has_async = sizeof...(a_indices) > 0;
            if constexpr (has_output && has_async)
            {
                auto result = instance(
                    typename async_output_t::async_edges(
                        tasks,
                        commit,
                        std::addressof(get<a_indices>(async_outputs))...
                    ),
                    get<i_indices>(inputs)->value()...
                );
                state = EState::Done;
                (get<s_indices>(sync_outputs).exec(std::move(get<s_indices>(result))), ...);
            }
            else if constexpr (has_output && !has_async)
            {
                auto result = instance(get<i_indices>(inputs)->value()...);
                state = EState::Done;
                (get<s_indices>(sync_outputs).exec(std::move(get<s_indices>(result))), ...);
            }
            else if constexpr (!has_output && has_async)
            {
                instance(
                    typename async_output_t::async_edges(
                        tasks,
                        commit,
                        std::addressof(get<a_indices>(async_outputs))...
                    ),
                    get<i_indices>(inputs)->value()...
                );
                state = EState::Done;
            }
            else if constexpr (!has_output && !has_async)
            {
                instance(get<i_indices>(inputs)->value()...);
                state = EState::Done;
            }
        }

        template <std::size_t... s_indices, std::size_t... a_indices>
        typename output_t::readonly_edges output_edges(
            std::index_sequence<s_indices...>,
            std::index_sequence<a_indices...> //
        )
        {
            return typename output_t::readonly_edges(
                std::addressof(get<s_indices>(sync_outputs))...,
                std::addressof(get<a_indices>(async_outputs))...
            );
        }

        Tasks* tasks;
        ICallable* commit;
        EState state;

        T instance;
        typename input_t::readonly_edges inputs;
        typename sync_output_t::edges sync_outputs;
        typename async_output_t::edges async_outputs;
    };
}
