#pragma once

#include <memory>
#include <optional>
#include <type_traits>

namespace nil::gate::detail
{
    template <typename T>
    struct edge_validate: std::true_type
    {
    };

    template <typename T>
    struct edge_validate<const T>: std::false_type
    {
    };

    template <typename T>
    struct edge_validate<T&>: std::false_type
    {
    };

    template <typename T>
    struct edge_validate<T*>: std::false_type
    {
    };

    template <typename T>
    struct edge_validate<std::optional<T>>: std::false_type
    {
    };

    template <typename T>
    struct edge_validate<std::unique_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct edge_validate<std::shared_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct edge_validate<std::unique_ptr<const T>>: std::true_type
    {
    };

    template <typename T>
    struct edge_validate<std::shared_ptr<const T>>: std::true_type
    {
    };

    template <typename T>
    struct edge_validate<std::optional<const T>>: std::true_type
    {
    };

    static_assert(edge_validate<bool>::value);

    static_assert(!edge_validate<bool&>::value);
    static_assert(!edge_validate<bool*>::value);

    static_assert(!edge_validate<const bool>::value);
    static_assert(!edge_validate<const bool* const>::value);
    static_assert(!edge_validate<const bool* const&>::value);
    static_assert(!edge_validate<const bool&>::value);

    static_assert(!edge_validate<std::unique_ptr<bool>>::value);
    static_assert(!edge_validate<std::shared_ptr<bool>>::value);
    static_assert(!edge_validate<std::optional<bool>>::value);

    static_assert(edge_validate<std::unique_ptr<const bool>>::value);
    static_assert(edge_validate<std::shared_ptr<const bool>>::value);
    static_assert(edge_validate<std::optional<const bool>>::value);
}
