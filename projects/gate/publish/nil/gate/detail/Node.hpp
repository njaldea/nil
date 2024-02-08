#pragma once

#include "INode.hpp"
#include "types.hpp"

namespace nil::gate::detail
{
    template <typename T>
    class Node final: public INode
    {
        using input_t = typename detail::traits<T>::i;
        using output_t = typename detail::traits<T>::o;

    public:
        template <typename... Args>
        Node(const typename input_t::readonly_edges& init_inputs, Args&&... args)
            : Node(
                  init_inputs,
                  typename input_t::type(),
                  typename input_t::make_index_sequence(),
                  typename output_t::make_index_sequence(),
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
                    typename output_t::make_index_sequence()
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

        typename output_t::readonly_edges output_edges()
        {
            return output_edges(typename output_t::make_index_sequence());
        }

    private:
        template <std::size_t>
        INode* spread_this()
        {
            return this;
        }

        template <
            typename... I,
            std::size_t... i_indices,
            std::size_t... o_indices,
            typename... Args>
        Node(
            const typename input_t::readonly_edges& init_inputs,
            nil::utils::traits::types<I...>,
            std::index_sequence<i_indices...>,
            std::index_sequence<o_indices...>,
            Args&&... args
        )
            : instance{std::forward<Args>(args)...}
            , inputs(init_inputs)
            , outputs(spread_this<o_indices>()...)
        {
            (static_cast<Edge<I>*>(std::get<i_indices>(inputs))->attach_output(this), ...);
        }

        template <std::size_t... o_indices>
        void pend(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs).pend(), ...);
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
                (std::get<o_indices>(outputs).exec(std::move(std::get<o_indices>(result))), ...);
            }
        }

        template <std::size_t... o_indices>
        typename output_t::readonly_edges output_edges(std::index_sequence<o_indices...>)
        {
            return
                typename output_t::readonly_edges(std::addressof(std::get<o_indices>(outputs))...);
        }

        State state = State::Pending;

        T instance;
        typename input_t::readonly_edges inputs;
        typename output_t::edges outputs;
    };
}
