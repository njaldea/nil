#pragma once

#include <tuple>
#include <type_traits>

#include "Edge.hpp"

namespace nil::gate::detail
{
    template <typename... T>
    struct types
    {
    };

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

    template <typename... I, typename... O>
    struct traits<std::tuple<O...>(I...)>
    {
        static constexpr std::size_t size_i = sizeof...(I);
        static constexpr std::size_t size_o = sizeof...(O);
        using type_i = types<std::decay_t<I>...>;
        using type_o = types<std::decay_t<O>...>;

        using edged_i = std::tuple<Edge<std::decay_t<I>>*...>;
        using edged_o = std::tuple<Edge<std::decay_t<O>>*...>;

        using redged_i = std::tuple<REdge<std::decay_t<I>>*...>;
        using redged_o = std::tuple<REdge<std::decay_t<O>>*...>;
    };
}
