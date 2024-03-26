#pragma once

#include "../traits/is_edge_type_valid.hpp"

namespace nil::gate::detail
{
    template <typename T, typename = void>
    struct is_equally_comparable: std::false_type
    {
    };

    template <typename T>
    struct is_equally_comparable<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>>
        : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_equally_comparable_v = is_equally_comparable<T>::value;

    template <typename T>
    constexpr bool edge_validate_v
        = gate::traits::is_edge_type_valid_v<T> && is_equally_comparable_v<T>;

    template <typename T>
    struct node_validate
    {
        static constexpr auto value //
            = edge_validate_v<T>    //
            && std::is_copy_constructible_v<T>;
    };

    template <typename T>
    struct node_validate<const T>
    {
        static constexpr auto value //
            = edge_validate_v<T>    //
            && std::is_copy_constructible_v<T>;
    };

    template <typename T>
    struct node_validate<const T&>
    {
        static constexpr auto value = edge_validate_v<T>;
    };

    template <typename T>
    struct node_validate<const T&&>
    {
        static constexpr auto value = edge_validate_v<T>;
    };

    template <typename T>
    struct node_validate<T&>
    {
        static constexpr auto value = false;
    };

    template <typename T>
    struct node_validate<T&&>
    {
        static constexpr auto value = false;
    };
}
