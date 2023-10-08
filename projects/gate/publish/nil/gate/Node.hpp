#pragma once

#include "INode.hpp"
#include "types.hpp"

namespace nil::gate
{
    template <typename T>
    class Node final: public INode
    {
    public:
        void exec() override
        {
            exec(
                std::make_index_sequence<detail::traits<T>::size_i>(),
                std::make_index_sequence<detail::traits<T>::size_o>()
            );
            status = State::Done;
        }

        void pend() override
        {
            if (status != State::Pending)
            {
                status = State::Pending;
                outputs_pend(std::make_index_sequence<detail::traits<T>::size_o>());
            }
        }

        template <std::size_t... o_indices>
        void outputs_pend(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs)->unset_value(), ...);
        }

        bool is_runnable() const override
        {
            return inputs_ready(std::make_index_sequence<detail::traits<T>::size_i>());
        }

        template <std::size_t... i_indices>
        bool inputs_ready(std::index_sequence<i_indices...>) const
        {
            return true && (std::get<i_indices>(inputs)->has_value() && ...);
        }

        template <std::size_t... i_indices, std::size_t... o_indices>
        auto exec(std::index_sequence<i_indices...>, std::index_sequence<o_indices...>)
        {
            if constexpr (sizeof...(o_indices) == 0)
            {
                instance(std::get<i_indices>(inputs)->value()...);
            }
            else
            {
                auto result = instance(std::get<i_indices>(inputs)->value()...);
                (std::get<o_indices>(outputs)->set_value(std::get<o_indices>(result)), ...);
            }
        }

        State state() const
        {
            return status;
        }

        template <std::uint32_t index, typename U>
        void attach_output(REdge<U>* edge)
        {
            std::get<index>(outputs) = down_cast(edge);
        }

    private:
        friend class Core;

        template <typename... Args>
        static std::unique_ptr<INode> create(
            typename detail::traits<T>::redged_i inputs,
            Args&&... args
        )
        {
            return std::unique_ptr<INode>(new Node<T>(
                std::move(inputs),
                std::make_index_sequence<detail::traits<T>::size_i>(),
                std::forward<Args>(args)...
            ));
        }

        template <typename U>
        static Edge<U>* down_cast(REdge<U>* edge)
        {
            return static_cast<Edge<U>*>(edge);
        }

        template <typename... Args, std::size_t... indices>
        Node(
            typename detail::traits<T>::redged_i inputs,
            std::index_sequence<indices...>,
            Args&&... args
        )
            : instance(std::forward<Args>(args)...)
            , inputs(down_cast(std::get<indices>(inputs))...)
        {
            if constexpr (sizeof...(indices) == 0)
            {
                (void)inputs;
            }
        }

        State status = State::Pending;

        T instance;
        typename detail::traits<T>::edged_i inputs;
        typename detail::traits<T>::edged_o outputs;
    };
}
