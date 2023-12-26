#pragma once

#include "INode.hpp"
#include "types.hpp"

namespace nil::gate::detail
{
    template <typename T>
    class Node final: public INode
    {
    public:
        template <typename... Args, std::size_t... i_indices>
        Node(
            typename detail::traits<T>::i::readonly_edges init_inputs,
            std::index_sequence<i_indices...>,
            Args&&... args
        )
            : instance(std::forward<Args>(args)...)
            , inputs(down_cast(std::get<i_indices>(init_inputs))...)
        {
            if constexpr (sizeof...(i_indices) == 0)
            {
                (void)init_inputs;
            }
        }

        void exec() override
        {
            if (state == State::Pending
                && is_runnable(typename detail::traits<T>::i::make_index_sequence()))
            {
                exec(
                    typename detail::traits<T>::i::make_index_sequence(),
                    typename detail::traits<T>::o::make_index_sequence()
                );
            }
        }

        void pend() override
        {
            if (state != State::Pending)
            {
                state = State::Pending;
                pend(typename detail::traits<T>::o::make_index_sequence());
            }
        }

        void cancel() override
        {
            if (state == State::Pending)
            {
                state = State::Cancelled;
                cancel(typename detail::traits<T>::o::make_index_sequence());
            }
        }

        template <std::uint32_t index, typename U>
        void attach_output(ReadOnlyEdge<U>* edge)
        {
            std::get<index>(outputs) = down_cast(edge);
        }

    private:
        template <typename U>
        static Edge<U>* down_cast(ReadOnlyEdge<U>* edge)
        {
            return static_cast<Edge<U>*>(edge);
        }

        template <std::size_t... o_indices>
        void pend(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs)->pend(), ...);
        }

        template <std::size_t... o_indices>
        void cancel(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs)->cancel(), ...);
        }

        template <std::size_t... i_indices>
        bool is_runnable(std::index_sequence<i_indices...>) const
        {
            return true && (... && std::get<i_indices>(inputs)->has_value());
        }

        template <std::size_t... i_indices, std::size_t... o_indices>
        auto exec(std::index_sequence<i_indices...>, std::index_sequence<o_indices...>)
        {
            if constexpr (sizeof...(o_indices) == 0)
            {
                instance(std::get<i_indices>(inputs)->value()...);
                state = State::Done;
            }
            else
            {
                auto result = instance(std::get<i_indices>(inputs)->value()...);
                state = State::Done;
                (std::get<o_indices>(outputs)->exec(std::move(std::get<o_indices>(result))), ...);
            }
        }

        State state = State::Pending;

        T instance;
        typename detail::traits<T>::i::edges inputs;
        typename detail::traits<T>::o::edges outputs;
    };
}
