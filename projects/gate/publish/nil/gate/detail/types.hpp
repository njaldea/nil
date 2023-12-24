#pragma once

#include "validation/edge.hpp"
#include "validation/node.hpp"

#include "Edge.hpp"

#include <tuple>

namespace nil::gate::detail
{
    template <typename... T>
    struct types
    {
        static constexpr auto size = sizeof...(T);
        using type = types<T...>;
        using edges = std::tuple<Edge<T>*...>;
        using readonly_edges = std::tuple<ReadOnlyEdge<T>*...>;
        using mutable_medges = std::tuple<MutableEdge<T>*...>;

        using make_sequence = std::make_index_sequence<size>;
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
        using i = types<typename edge_validate<std::decay_t<I>>::type...>;
        using o = types<typename edge_validate<std::decay_t<O>>::type...>;
        static constexpr bool is_valid =                    //
            (true && (... && node_validate_i<I>::value)) && //
            (true && (... && node_validate_o<O>::value));
    };
}
