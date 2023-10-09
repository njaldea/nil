#pragma once

#include "INode.hpp"
#include "types.hpp"

namespace nil::gate
{
    class Core;
}

namespace nil::gate::detail
{
    template <typename T>
    class Node final: public INode
    {
    public:
        void exec() override
        {
            exec(
                typename detail::traits<T>::i::make_sequence(),
                typename detail::traits<T>::o::make_sequence()
            );
        }

        void pend() override
        {
            if (status != State::Pending)
            {
                status = State::Pending;
                outputs_pend(typename detail::traits<T>::o::make_sequence());
            }
        }

        void cancel() override
        {
            if (status == State::Pending)
            {
                status = State::Cancelled;
                outputs_cancel(typename detail::traits<T>::o::make_sequence());
            }
        }

        bool is_runnable() const override
        {
            return inputs_ready(typename detail::traits<T>::i::make_sequence());
        }

        State state() const override
        {
            return status;
        }

        template <std::uint32_t index, typename U>
        void attach_output(REdge<U>* edge)
        {
            std::get<index>(outputs) = down_cast(edge);
        }

    private:
        friend Core;

        template <typename... Args, std::size_t... i_indices>
        Node(
            typename detail::traits<T>::i::redged inputs,
            std::index_sequence<i_indices...>,
            Args&&... args
        )
            : instance(std::forward<Args>(args)...)
            , inputs(down_cast(std::get<i_indices>(inputs))...)
        {
            if constexpr (sizeof...(i_indices) == 0)
            {
                (void)inputs;
            }
        }

        template <typename... Args, std::size_t... i_indices>
        static std::unique_ptr<INode> create(
            typename detail::traits<T>::i::redged inputs,
            std::index_sequence<i_indices...> indices,
            Args&&... args
        )
        {
            return std::unique_ptr<INode>(
                new Node<T>(std::move(inputs), indices, std::forward<Args>(args)...)
            );
        }

        template <typename U>
        static Edge<U>* down_cast(REdge<U>* edge)
        {
            return static_cast<Edge<U>*>(edge);
        }

        template <std::size_t... o_indices>
        void outputs_pend(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs)->pend(), ...);
        }

        template <std::size_t... o_indices>
        void outputs_cancel(std::index_sequence<o_indices...>)
        {
            (std::get<o_indices>(outputs)->cancel(), ...);
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
                status = State::Done;
            }
            else
            {
                auto result = instance(std::get<i_indices>(inputs)->value()...);
                status = State::Done;
                (std::get<o_indices>(outputs)->exec(std::move(std::get<o_indices>(result))), ...);
            }
        }

        State status = State::Pending;

        T instance;
        typename detail::traits<T>::i::edged inputs;
        typename detail::traits<T>::o::edged outputs;
    };
}
