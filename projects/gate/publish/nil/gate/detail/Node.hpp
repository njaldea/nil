#pragma once

#include "INode.hpp"
#include "Tasks.hpp"
#include "types.hpp"

namespace nil::gate::detail
{
    template <typename T, typename = void>
    class Node final: public INode
    {
        using input_t = typename detail::traits<T>::i;
        using output_t = typename detail::traits<T>::o;
        using async_t = typename detail::traits<T>::a;
        using real_out_t = typename detail::traits<T>::outs;

    public:
        template <typename... Args>
        Node(
            Tasks* init_tasks,
            ICallable* init_commit,
            const typename input_t::readonly_edges& init_inputs,
            typename async_t::tuple init_asyncs,
            Args&&... args
        )
            : Node(
                  init_tasks,
                  init_commit,
                  init_inputs,
                  std::move(init_asyncs),
                  typename input_t::type(),
                  typename input_t::make_index_sequence(),
                  typename output_t::make_index_sequence(),
                  typename async_t::make_index_sequence(),
                  std::forward<Args>(args)...
              )
        {
        }

        void exec() override
        {
            if (state == State::Pending)
            {
                exec(
                    typename input_t::make_index_sequence(),
                    typename output_t::make_index_sequence(),
                    typename async_t::make_index_sequence()
                );
            }
        }

        void pend() override
        {
            if (state != State::Pending)
            {
                state = State::Pending;
                pend(typename output_t::make_index_sequence());
            }
        }

        typename real_out_t::readonly_edges output_edges()
        {
            return output_edges(
                typename output_t::make_index_sequence(),
                typename async_t::make_index_sequence()
            );
        }

    private:
        template <
            typename... I,
            std::size_t... i_indices,
            std::size_t... o_indices,
            std::size_t... a_indices,
            typename... Args>
        Node(
            Tasks* init_tasks,
            ICallable* init_commit,
            const typename input_t::readonly_edges& init_inputs,
            [[maybe_unused]] typename async_t::tuple init_asyncs,
            nil::utils::traits::types<I...>,
            std::index_sequence<i_indices...>,
            std::index_sequence<o_indices...>,
            std::index_sequence<a_indices...>,
            Args&&... args
        )
            : tasks(init_tasks)
            , commit(init_commit)
            , state(State::Pending)
            , instance{std::forward<Args>(args)...}
            , inputs(init_inputs)
            , outputs()
            , asyncs()
        {
            (static_cast<InternalEdge<I>*>(std::get<i_indices>(inputs))->attach_output(this), ...);
            (std::get<o_indices>(outputs).attach_input(this), ...);
            (std::get<o_indices>(outputs).attach_tasks(tasks), ...);
            (std::get<a_indices>(asyncs).attach_input(this), ...);
            (std::get<a_indices>(asyncs).attach_tasks(tasks), ...);
            (std::get<a_indices>(asyncs).exec(std::move(std::get<a_indices>(init_asyncs))), ...);
        }

        template <std::size_t... o_indices>
        void pend(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs).pend(), ...);
        }

        template <std::size_t... i_indices, std::size_t... o_indices, std::size_t... a_indices>
        auto exec(
            std::index_sequence<i_indices...>,
            std::index_sequence<o_indices...>,
            std::index_sequence<a_indices...> //
        )
        {
            constexpr auto has_output = sizeof...(o_indices) > 0;
            constexpr auto has_async = sizeof...(a_indices) > 0;
            if constexpr (has_output && has_async)
            {
                auto result = instance(
                    typename async_t::async_edges(
                        tasks,
                        commit,
                        std::addressof(std::get<a_indices>(asyncs))...
                    ),
                    std::get<i_indices>(inputs)->value()...
                );
                state = State::Done;
                (std::get<o_indices>(outputs).exec(std::move(std::get<o_indices>(result))), ...);
            }
            else if constexpr (has_output && !has_async)
            {
                auto result = instance(std::get<i_indices>(inputs)->value()...);
                state = State::Done;
                (std::get<o_indices>(outputs).exec(std::move(std::get<o_indices>(result))), ...);
            }
            else if constexpr (!has_output && has_async)
            {
                instance(
                    typename async_t::async_edges(
                        tasks,
                        commit,
                        std::addressof(std::get<a_indices>(asyncs))...
                    ),
                    std::get<i_indices>(inputs)->value()...
                );
                state = State::Done;
            }
            else if constexpr (!has_output && !has_async)
            {
                instance(std::get<i_indices>(inputs)->value()...);
                state = State::Done;
            }
        }

        template <std::size_t... o_indices, std::size_t... a_indices>
        typename real_out_t::readonly_edges
            output_edges(std::index_sequence<o_indices...>, std::index_sequence<a_indices...>)
        {
            return typename real_out_t::readonly_edges(
                std::addressof(std::get<o_indices>(outputs))...,
                std::addressof(std::get<a_indices>(asyncs))...
            );
        }

        Tasks* tasks;
        ICallable* commit;
        State state;

        T instance;
        typename input_t::readonly_edges inputs;
        typename output_t::edges outputs;
        typename async_t::edges asyncs;
    };
}
