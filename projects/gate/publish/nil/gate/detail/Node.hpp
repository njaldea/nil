#pragma once

#include "INode.hpp"
#include "Tasks.hpp"
#include "callable_traits.hpp"

namespace nil::gate::detail
{
    template <typename T, typename = void>
    class Node final: public INode
    {
        using input_t = typename detail::callable_traits<T>::inputs;
        using sync_output_t = typename detail::callable_traits<T>::sync_outputs;
        using async_output_t = typename detail::callable_traits<T>::async_outputs;
        using output_t = typename detail::callable_traits<T>::outputs;

    public:
        template <typename... Args>
        Node(
            Tasks* init_tasks,
            const typename input_t::readonly_edges& init_inputs,
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

        void exec(ICallable* callable) override
        {
            if (state == EState::Pending)
            {
                if constexpr (callable_traits<T>::has_async)
                {
                    auto result = call(callable);
                    state = EState::Done;
                    forward(result);
                }
                else
                {
                    call(callable);
                    state = EState::Done;
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
            const typename input_t::readonly_edges& init_inputs,
            [[maybe_unused]] typename async_output_t::tuple init_asyncs,
            nil::utils::traits::types<I...>,
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
            (static_cast<InternalEdge<I>*>(get<i_indices>(inputs))->attach_output(this), ...);
            (get<s_indices>(sync_outputs).attach_tasks(tasks), ...);
            (get<a_indices>(async_outputs).attach_tasks(tasks), ...);
            (get<a_indices>(async_outputs).exec(std::move(get<a_indices>(init_asyncs))), ...);
        }

        template <std::size_t... s_indices>
        void pend(std::index_sequence<s_indices...>)
        {
            (get<s_indices>(sync_outputs).pend(), ...);
        }

        auto call(ICallable* commit)
        {
            using return_t = decltype(std::declval<Node<T>>()
                                          .call(nullptr, typename input_t::make_index_sequence()));
            constexpr auto is_void_return = std::is_same_v<void, return_t>;
            if constexpr (is_void_return)
            {
                call(commit, typename input_t::make_index_sequence());
                return std::tuple<>();
            }
            else
            {
                return call(commit, typename input_t::make_index_sequence());
            }
        }

        auto forward(sync_output_t::tuple& result)
        {
            forward(result, typename sync_output_t::make_index_sequence());
        }

        auto async_edges(ICallable* commit)
        {
            return async_edges(commit, typename async_output_t::make_index_sequence());
        }

        template <std::size_t... i_indices>
        auto call(ICallable* commit, std::index_sequence<i_indices...>)
        {
            if constexpr (callable_traits<T>::has_async)
            {
                return instance(async_edges(commit), get<i_indices>(inputs)->value()...);
            }
            else
            {
                return instance(get<i_indices>(inputs)->value()...);
            }
        }

        template <std::size_t... s_indices>
        auto forward(sync_output_t::tuple& result, std::index_sequence<s_indices...>)
        {
            (get<s_indices>(sync_outputs).exec(std::move(get<s_indices>(result))), ...);
        }

        template <std::size_t... a_indices>
        auto async_edges(ICallable* commit, std::index_sequence<a_indices...>)
        {
            return typename async_output_t::async_edges(
                tasks,
                commit,
                std::addressof(get<a_indices>(async_outputs))...
            );
        }

        template <std::size_t... s_indices, std::size_t... a_indices>
        auto output_edges(std::index_sequence<s_indices...>, std::index_sequence<a_indices...>)
        {
            return typename output_t::readonly_edges(
                std::addressof(get<s_indices>(sync_outputs))...,
                std::addressof(get<a_indices>(async_outputs))...
            );
        }

        Tasks* tasks;
        EState state;

        T instance;
        typename input_t::readonly_edges inputs;
        typename sync_output_t::edges sync_outputs;
        typename async_output_t::edges async_outputs;
    };
}
