#pragma once

#include "../traits/is_edge_type_valid.hpp"

#include <functional>
#include <memory>
#include <optional>

namespace nil::gate::traits
{
    template <typename T>
    struct is_edge_type_valid<std::optional<T>>: std::false_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::unique_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::shared_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::reference_wrapper<T>>: std::false_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::unique_ptr<const T>>: std::true_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::shared_ptr<const T>>: std::true_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::optional<const T>>: std::true_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::reference_wrapper<const T>>: std::true_type
    {
    };
};
