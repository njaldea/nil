#pragma once

#include "../Core.hpp"

namespace nil::gate::api::uniform
{
    template <typename T>
    auto add_node(
        T instance,
        nil::gate::Core& core,
        typename nil::gate::detail::traits::node<T>::async_outputs::tuple async_initializers,
        typename nil::gate::detail::traits::node<T>::inputs::edges inputs
    )
    {
        constexpr auto has_output = nil::gate::detail::traits::node<T>::outputs::size > 0;
        constexpr auto has_input = nil::gate::detail::traits::node<T>::inputs::size > 0;
        constexpr auto has_async = nil::gate::detail::traits::node<T>::async_outputs::size > 0;

        if constexpr (has_output && has_input && has_async)
        {
            return core.node(std::move(instance), std::move(async_initializers), inputs);
        }
        if constexpr (has_output && has_input && !has_async)
        {
            return core.node(std::move(instance), inputs);
        }
        if constexpr (has_output && !has_input && has_async)
        {
            return core.node(std::move(instance), std::move(async_initializers));
        }
        if constexpr (has_output && !has_input && !has_async)
        {
            return core.node(std::move(instance));
        }
        if constexpr (!has_output && has_input && has_async)
        {
            core.node(std::move(instance), std::move(async_initializers), inputs);
            return std::tuple<>();
        }
        if constexpr (!has_output && has_input && !has_async)
        {
            core.node(std::move(instance), inputs);
            return std::tuple<>();
        }
        if constexpr (!has_output && !has_input && has_async)
        {
            core.node(std::move(instance), std::move(async_initializers));
            return std::tuple<>();
        }
        if constexpr (!has_output && !has_input && !has_async)
        {
            core.node(std::move(instance));
            return std::tuple<>();
        }
    }
}
