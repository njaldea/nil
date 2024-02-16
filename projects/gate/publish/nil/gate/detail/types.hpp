#pragma once

#include "validation/edge.hpp"
#include "validation/node.hpp"

#include "Edge.hpp"

#include <nil/utils/traits/callable.hpp>

namespace nil::gate::detail
{
    template <typename... T>
    struct types
    {
        static constexpr auto size = sizeof...(T);
        using type = nil::utils::traits::types<T...>;
        using initializer = std::tuple<T...>;
        using edges = std::tuple<Edge<T>...>;
        using readonly_edges = std::tuple<ReadOnlyEdge<T>*...>;
        using mutable_edges = std::tuple<MutableEdge<T>*...>;
        using make_index_sequence = std::make_index_sequence<size>;
    };

    template <typename T>
    struct traits: traits<typename nil::utils::traits::callable<T>::type>
    {
    };

    template <typename... I, typename... O>
    struct traits<nil::utils::traits::types<O...>(I...)>
    {
        using i = types<typename edge_validate<std::decay_t<I>>::type...>;
        using o = types<typename edge_validate<std::decay_t<O>>::type...>;
        using a = types<>;
        using outs = o;
        static constexpr bool has_async = false;
        static constexpr bool is_valid                     //
            = (true && (... && node_validate_i<I>::value)) //
            && (true && (... && node_validate_o<O>::value));
    };

    template <typename... I, typename... O, typename... A>
    struct traits<nil::utils::traits::types<O...>(std::tuple<MutableEdge<A>*...>, I...)>
    {
        using i = types<typename edge_validate<std::decay_t<I>>::type...>;
        using o = types<typename edge_validate<std::decay_t<O>>::type...>;
        using a = types<typename edge_validate<std::decay_t<A>>::type...>;
        using outs = types<typename edge_validate<std::decay_t<O>>::type..., A...>;
        static constexpr bool has_async = true;
        static constexpr bool is_valid                      //
            = (true && (... && node_validate_i<I>::value))  //
            && (true && (... && node_validate_o<O>::value)) //
            && (true && (... && edge_validate<A>::value));
    };
}
