#pragma once

#include "../../traits/is_edge_type_valid.hpp"

#include <type_traits>

namespace nil::gate::detail
{
    template <typename T>
    struct node_validate
    {
        static constexpr auto value                      //
            = nil::gate::traits::is_edge_type_valid_v<T> //
            && std::is_copy_constructible_v<T>;
    };

    template <typename T>
    struct node_validate<const T>
    {
        static constexpr auto value                      //
            = nil::gate::traits::is_edge_type_valid_v<T> //
            && std::is_copy_constructible_v<T>;
    };

    template <typename T>
    struct node_validate<const T&>
    {
        static constexpr auto value = nil::gate::traits::is_edge_type_valid_v<T>;
    };

    template <typename T>
    struct node_validate<const T&&>
    {
        static constexpr auto value = nil::gate::traits::is_edge_type_valid_v<T>;
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
