#pragma once

#include <tuple>
#include <type_traits>

#include "Edge.hpp"

namespace nil::gate::detail
{
    template <typename T>
    struct traits: traits<decltype(&T::operator())>
    {
    };

    template <typename T, typename R, typename... Args>
    struct traits<R (T::*)(Args...) const>: traits<R(Args...)>
    {
    };

    template <typename T, typename R, typename... Args>
    struct traits<R (T::*)(Args...)>: traits<R(Args...)>
    {
    };

    template <typename... I>
    struct traits<void(I...)>: traits<std::tuple<>(I...)>
    {
    };

    template <typename... T>
    struct types
    {
        static constexpr auto size = sizeof...(T);
        using type = types<std::decay_t<T>...>;
        using edged = std::tuple<Edge<std::decay_t<T>>*...>;
        using redged = std::tuple<REdge<std::decay_t<T>>*...>;
        using medged = std::tuple<MEdge<std::decay_t<T>>*...>;

        using make_sequence = std::make_index_sequence<size>;
    };

    template <typename... I, typename... O>
    struct traits<std::tuple<O...>(I...)>
    {
        using i = types<I...>;
        using o = types<O...>;
    };
}
