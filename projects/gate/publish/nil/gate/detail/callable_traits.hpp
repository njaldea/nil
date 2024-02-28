#pragma once

#include "validation/edge.hpp"
#include "validation/node.hpp"

#include "edge/Async.hpp"
#include "edge/Data.hpp"

#include <nil/utils/traits/callable.hpp>

namespace nil::gate::detail
{
    template <typename... T>
    struct common_traits
    {
        static constexpr auto size = sizeof...(T);
        using make_index_sequence = std::make_index_sequence<size>;
        using type = nil::utils::traits::types<T...>;
    };

    template <typename... T>
    struct input_traits final: common_traits<T...>
    {
        using readonly_edges = std::tuple<ReadOnlyEdge<T>*...>;
    };

    template <typename... T>
    struct sync_output_traits final: common_traits<T...>
    {
        using tuple = std::tuple<T...>;
        using edges = std::tuple<DataEdge<T>...>;
        using readonly_edges = std::tuple<ReadOnlyEdge<T>*...>;
    };

    template <typename... T>
    struct async_output_traits final: common_traits<T...>
    {
        using tuple = std::tuple<T...>;
        using edges = std::tuple<DataEdge<T>...>;
        using async_edges = nil::gate::detail::AsyncEdges<T...>;
    };

    template <typename... T>
    struct output_traits final: common_traits<T...>
    {
        using readonly_edges = std::tuple<ReadOnlyEdge<T>*...>;
    };

    template <typename T>
    struct traits;

    template <typename... I, typename... S>
    struct traits<nil::utils::traits::types<S...>(I...)>
    {
        using inputs = input_traits<typename edge_validate<std::decay_t<I>>::type...>;
        using sync_outputs = sync_output_traits<typename edge_validate<std::decay_t<S>>::type...>;
        using async_outputs = async_output_traits<>;
        using outputs = output_traits<typename edge_validate<std::decay_t<S>>::type...>;
        static constexpr bool has_async = false;
        static constexpr bool is_valid                   //
            = (true && ... && node_validate_i<I>::value) //
            && (true && ... && node_validate_s<S>::value);
    };

    template <typename... I, typename... S, typename... A>
    struct traits<nil::utils::traits::types<S...>(async_edges<A...>, I...)>
        : traits<nil::utils::traits::types<S...>(const async_edges<A...>&, I...)>
    {
    };

    template <typename... I, typename... S, typename... A>
    struct traits<nil::utils::traits::types<S...>(async_edges<A...>&, I...)>
    {
        using inputs = input_traits<typename edge_validate<std::decay_t<I>>::type...>;
        using sync_outputs = sync_output_traits<typename edge_validate<std::decay_t<S>>::type...>;
        using async_outputs = async_output_traits<typename edge_validate<std::decay_t<A>>::type...>;
        using outputs = output_traits<
            typename edge_validate<std::decay_t<S>>::type...,
            typename edge_validate<std::decay_t<A>>::type...>;
        static constexpr bool has_async = true;
        static constexpr bool is_valid = false;
    };

    template <typename... I, typename... S, typename... A>
    struct traits<nil::utils::traits::types<S...>(const async_edges<A...>&, I...)>
    {
        using inputs = input_traits<typename edge_validate<std::decay_t<I>>::type...>;
        using sync_outputs = sync_output_traits<typename edge_validate<std::decay_t<S>>::type...>;
        using async_outputs = async_output_traits<typename edge_validate<std::decay_t<A>>::type...>;
        using outputs = output_traits<
            typename edge_validate<std::decay_t<S>>::type...,
            typename edge_validate<std::decay_t<A>>::type...>;
        static constexpr bool has_async = true;
        static constexpr bool is_valid                    //
            = (true && ... && node_validate_i<I>::value)  //
            && (true && ... && node_validate_s<S>::value) //
            && (true && ... && edge_validate<A>::value);
    };

    template <typename T>
    using callable_traits = traits<typename nil::utils::traits::callable<T>::type>;
}
