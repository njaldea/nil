#pragma once

#include "validation/edge.hpp"
#include "validation/node.hpp"

#include "edge/Async.hpp"
#include "edge/Data.hpp"

#include <nil/utils/traits/callable.hpp>

namespace nil::gate::detail
{
    template <typename... T>
    struct types
    {
        static constexpr auto size = sizeof...(T);
        using type = nil::utils::traits::types<T...>;
        using tuple = std::tuple<T...>;
        using edges = std::tuple<DataEdge<T>...>;
        using async_edges = nil::gate::detail::AsyncEdges<T...>;
        using readonly_edges = std::tuple<ReadOnlyEdge<T>*...>;
        using mutable_edges = std::tuple<MutableEdge<T>*...>;
        using internal_edges = std::tuple<InternalEdge<T>*...>;
        using make_index_sequence = std::make_index_sequence<size>;
    };

    template <typename T>
    struct traits: traits<typename nil::utils::traits::callable<T>::type>
    {
    };

    template <typename... I, typename... O>
    struct traits<nil::utils::traits::types<O...>(I...)>
    {
        using inputs = types<typename edge_validate<std::decay_t<I>>::type...>;
        using sync_outputs = types<typename edge_validate<std::decay_t<O>>::type...>;
        using async_outputs = types<>;
        using all_outputs = sync_outputs;
        static constexpr bool has_async = false;
        static constexpr bool is_valid                     //
            = (true && (... && node_validate_i<I>::value)) //
            && (true && (... && node_validate_o<O>::value));
    };

    template <typename... I, typename... O, typename... A>
    struct traits<nil::utils::traits::types<O...>(async_edges<A...>, I...)>
        : traits<nil::utils::traits::types<O...>(const async_edges<A...>&, I...)>
    {
    };

    template <typename... I, typename... O, typename... A>
    struct traits<nil::utils::traits::types<O...>(const async_edges<A...>&, I...)>
    {
        using inputs = types<typename edge_validate<std::decay_t<I>>::type...>;
        using sync_outputs = types<typename edge_validate<std::decay_t<O>>::type...>;
        using async_outputs = types<typename edge_validate<std::decay_t<A>>::type...>;
        using all_outputs = types<typename edge_validate<std::decay_t<O>>::type..., A...>;
        static constexpr bool has_async = true;
        static constexpr bool is_valid                      //
            = (true && (... && node_validate_i<I>::value))  //
            && (true && (... && node_validate_o<O>::value)) //
            && (true && (... && edge_validate<A>::value));
    };
}
