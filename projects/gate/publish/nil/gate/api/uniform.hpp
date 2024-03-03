#pragma once

#include "../Core.hpp"

namespace nil::gate::api::uniform
{
    template <typename T, typename... Args>
    auto add_node(
        nil::gate::Core& core,
        typename nil::gate::detail::traits::node<T>::async_outputs::tuple async_initializers,
        typename nil::gate::detail::traits::node<T>::inputs::edges inputs,
        Args&&... args
    )
    {
        constexpr auto has_output = nil::gate::detail::traits::node<T>::outputs::size > 0;
        constexpr auto has_input = nil::gate::detail::traits::node<T>::inputs::size > 0;
        constexpr auto has_async = nil::gate::detail::traits::node<T>::async_outputs::size > 0;

        if constexpr (has_output && has_input && has_async)
        {
            return core.node<T>(std::move(async_initializers), inputs, args...);
        }
        else if constexpr (has_output && has_input && !has_async)
        {
            return core.node<T>(inputs, args...);
        }
        else if constexpr (has_output && !has_input && has_async)
        {
            return core.node<T>(std::move(async_initializers), args...);
        }
        else if constexpr (has_output && !has_input && !has_async)
        {
            return core.node<T>(args...);
        }
        else if constexpr (!has_output && has_input && has_async)
        {
            core.node<T>(std::move(async_initializers), inputs, args...);
            return std::tuple<>();
        }
        else if constexpr (!has_output && has_input && !has_async)
        {
            core.node<T>(inputs, args...);
            return std::tuple<>();
        }
        else if constexpr (!has_output && !has_input && has_async)
        {
            core.node<T>(std::move(async_initializers), args...);
            return std::tuple<>();
        }
        else if constexpr (!has_output && !has_input && !has_async)
        {
            core.node<T>(args...);
            return std::tuple<>();
        }
    }
}
