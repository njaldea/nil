#pragma once

#include "../../traits/edgify.hpp"
#include "../../traits/is_edge_type_valid.hpp"
#include "../../types.hpp"
#include "../DataEdge.hpp"
#include "../validation/node.hpp"
#include "callable.hpp"

#include <type_traits>

namespace nil::gate
{
    class Core;
}

namespace nil::gate::detail::traits
{
    template <typename T>
    using edgify_t = gate::traits::edgify_t<T>;
    template <typename T>
    using is_edge_type_valid = gate::traits::is_edge_type_valid<T>;

    template <typename A>
    struct async_checker
    {
        static constexpr bool is_async = false;
    };

    template <typename... A>
    struct async_checker<nil::gate::async_outputs<A...>>
    {
        static constexpr bool is_async = true;
    };

    template <typename A>
    struct async_resolver;

    template <typename... A>
    struct async_resolver<nil::gate::async_outputs<A...>>
    {
        using types = types<A...>;
    };

    template <typename I>
    struct input_resolver;

    template <typename... I>
    struct input_resolver<types<I...>>
    {
        using inputs = types<I...>;
        using asyncs = types<>;
        static constexpr bool has_async = false;
        static constexpr bool has_core = false;
        static constexpr bool is_valid = true;
    };

    template <typename T>
    static constexpr bool is_const_reference_v
        = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;
    template <typename T>
    static constexpr bool is_vanilla_v = std::is_same_v<T, std::decay_t<T>>;

    template <typename First, typename... I>
        requires async_checker<std::decay_t<First>>::is_async
    struct input_resolver<types<First, I...>>
    {
        using inputs = types<I...>;
        using asyncs = async_resolver<std::decay_t<First>>::types;
        static constexpr bool has_async = true;
        static constexpr bool has_core = false;
        static constexpr bool is_valid //
            = is_vanilla_v<First> || is_const_reference_v<First>;
    };

    template <typename First, typename Second, typename... I>
        requires async_checker<std::decay_t<First>>::is_async
        && std::is_same_v<nil::gate::Core, std::decay_t<Second>>
    struct input_resolver<types<First, Second, I...>>
    {
        using inputs = types<I...>;
        using asyncs = async_resolver<std::decay_t<First>>::types;
        static constexpr bool has_async = true;
        static constexpr bool has_core = true;
        static constexpr bool is_valid     //
            = is_const_reference_v<Second> //
            && (is_vanilla_v<First> || is_const_reference_v<First>);
    };

    template <typename T>
    struct node_inputs;

    template <typename... I>
    struct node_inputs<types<I...>>
    {
        using type = types<edgify_t<std::decay_t<I>>...>;
        using edges = nil::gate::inputs<edgify_t<std::decay_t<I>>...>;
        using make_index_sequence = std::make_index_sequence<sizeof...(I)>;
        static constexpr auto size = sizeof...(I);
        static constexpr bool is_valid = (true && ... && node_validate<I>::value);
    };

    template <typename T>
    struct node_sync_outputs;

    template <typename... S>
    struct node_sync_outputs<types<S...>>
    {
        using type = types<edgify_t<std::decay_t<S>>...>;
        using tuple = std::tuple<edgify_t<std::decay_t<S>>...>;
        using edges = nil::gate::sync_outputs<edgify_t<std::decay_t<S>>...>;
        using data_edges = std::tuple<detail::edges::Data<edgify_t<std::decay_t<S>>>...>;
        using make_index_sequence = std::make_index_sequence<sizeof...(S)>;
        static constexpr auto size = sizeof...(S);
        static constexpr bool is_valid
            = (true && ... && (!std::is_reference_v<S> && !std::is_pointer_v<S>));
    };

    template <typename T>
    struct node_async_outputs;

    template <typename... A>
    struct node_async_outputs<types<A...>>
    {
        using type = types<edgify_t<std::decay_t<A>>...>;
        using tuple = std::tuple<edgify_t<std::decay_t<A>>...>;
        using edges = nil::gate::async_outputs<edgify_t<std::decay_t<A>>...>;
        using data_edges = std::tuple<detail::edges::Data<edgify_t<std::decay_t<A>>>...>;
        using make_index_sequence = std::make_index_sequence<sizeof...(A)>;
        static constexpr auto size = sizeof...(A);
        static constexpr bool is_valid = (true && ... && is_edge_type_valid<A>::value);
    };

    template <typename S, typename A>
    struct node_outputs;

    template <typename... S, typename... A>
    struct node_outputs<types<S...>, types<A...>>
    {
        using type = types<edgify_t<std::decay_t<S>>..., edgify_t<std::decay_t<A>>...>;
        using edges = std::conditional_t<
            sizeof...(S) + sizeof...(A) == 0,
            void,
            nil::gate::outputs<
                edgify_t<std::decay_t<S>>...,
                edgify_t<std::decay_t<A>>...> //
            >;
        using make_index_sequence = std::make_index_sequence<sizeof...(S) + sizeof...(A)>;
        static constexpr auto size = sizeof...(S) + sizeof...(A);
    };

    template <typename T>
    struct node final
    {
        using full_i = typename callable<T>::inputs;
        using final_s = typename callable<T>::outputs;
        using final_i = typename input_resolver<full_i>::inputs;
        using final_a = typename input_resolver<full_i>::asyncs;

        using inputs = node_inputs<final_i>;
        using sync_outputs = node_sync_outputs<final_s>;
        using async_outputs = node_async_outputs<final_a>;
        using outputs = node_outputs<final_s, final_a>;

        using input_resolver_t = input_resolver<full_i>;
        static constexpr bool has_async = input_resolver_t::has_async;
        static constexpr bool has_core = input_resolver_t::has_core;
        static constexpr bool is_valid   //
            = input_resolver_t::is_valid //
            && inputs::is_valid          //
            && sync_outputs::is_valid    //
            && async_outputs::is_valid;
    };
}
