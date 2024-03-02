#pragma once

#include "../validation/edge.hpp"
#include "../validation/node.hpp"

#include "../DataEdge.hpp"

#include "../../types.hpp"

#include "callable.hpp"

namespace nil::gate
{
    class Core;
}

namespace nil::gate::detail::traits
{
    template <typename... T>
    struct node_common
    {
        static constexpr auto size = sizeof...(T);
        using make_index_sequence = std::make_index_sequence<size>;
        using type = types<T...>;
    };

    template <typename... T>
    struct node_inputs
    {
        struct inputs final: node_common<edgify_t<T>...>
        {
            using edges = nil::gate::inputs<edgify_t<T>...>;
        };

        static constexpr bool has_core = false;
        static constexpr bool is_input_valid = (true && ... && node_validate_i<T>::value);
    };

    template <typename... T>
    struct node_inputs<const Core&, T...>
    {
        struct inputs final: node_common<edgify_t<T>...>
        {
            using edges = nil::gate::inputs<edgify_t<T>...>;
        };

        static constexpr bool has_core = true;
        static constexpr bool is_input_valid = (true && ... && node_validate_i<T>::value);
    };

    template <typename... T>
    struct node_inputs<Core&, T...>
    {
        struct inputs final: node_common<edgify_t<T>...>
        {
            using edges = nil::gate::inputs<edgify_t<T>...>;
        };

        static constexpr bool has_core = true;
        static constexpr bool is_input_valid = false;
    };

    template <typename... T>
    struct node_inputs<Core, T...>
    {
        struct inputs final: node_common<edgify_t<T>...>
        {
            using edges = nil::gate::inputs<edgify_t<T>...>;
        };

        static constexpr bool has_core = true;
        static constexpr bool is_input_valid = false;
    };

    template <typename... T>
    struct node_outputs_sync
    {
        struct sync_outputs final: node_common<edgify_t<T>...>
        {
            using tuple = std::tuple<edgify_t<T>...>;
            using data_edges = std::tuple<DataEdge<edgify_t<T>>...>;
            using edges = nil::gate::sync_outputs<edgify_t<T>...>;
        };
    };

    template <typename... T>
    struct node_outputs_async
    {
        struct async_outputs final: node_common<edgify_t<T>...>
        {
            using tuple = std::tuple<edgify_t<T>...>;
            using data_edges = std::tuple<DataEdge<edgify_t<T>>...>;
            using edges = nil::gate::async_outputs<edgify_t<T>...>;
        };
    };

    template <typename... T>
    struct node_outputs
    {
        struct outputs final: node_common<edgify_t<T>...>
        {
            using edges = nil::gate::outputs<edgify_t<T>...>;
        };
    };

    template <>
    struct node_outputs<>
    {
        struct outputs final: node_common<>
        {
            using edges = void;
        };
    };

    template <typename T>
    struct node: node<typename callable<T>::type>
    {
    };

    template <typename... I, typename... S>
    struct node<types<S...>(I...)>
        : node_inputs<I...>
        , node_outputs_sync<S...>
        , node_outputs_async<>
        , node_outputs<S...>
    {
        using self_t = node<types<S...>(I...)>;
        static constexpr bool has_async = false;
        static constexpr bool is_valid //
            = self_t::is_input_valid   //
            && !self_t::has_core       //
            && (true && ... && node_validate_s<S>::value);
    };

    template <typename... I, typename... S, typename... A>
    struct node<types<S...>(async_outputs<A...>, I...)>
        : node<types<S...>(const async_outputs<A...>&, I...)>
    {
    };

    template <typename... I, typename... S, typename... A>
    struct node<types<S...>(async_outputs<A...>&, I...)>
        : node_inputs<I...>
        , node_outputs_sync<S...>
        , node_outputs_async<A...>
        , node_outputs<S..., A...>
    {
        static constexpr bool has_async = true;
        static constexpr bool is_valid = false;
    };

    template <typename... I, typename... S, typename... A>
    struct node<types<S...>(const async_outputs<A...>&, I...)>
        : node_inputs<I...>
        , node_outputs_sync<S...>
        , node_outputs_async<A...>
        , node_outputs<S..., A...>
    {
        using self_t = node<types<S...>(const async_outputs<A...>&, I...)>;
        static constexpr bool has_async = true;
        static constexpr bool is_valid                    //
            = self_t::is_input_valid                      //
            && sizeof...(A) > 0                           //
            && (true && ... && node_validate_s<S>::value) //
            && (true && ... && edge_validate<A>::value);
    };
}
