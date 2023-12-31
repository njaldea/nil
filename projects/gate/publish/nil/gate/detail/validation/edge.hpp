#pragma once

#include <memory>
#include <type_traits>

namespace nil::gate::detail
{
    template <typename T>
    struct edge_validate: std::true_type
    {
        using type = T;
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
    struct edge_validate<std::unique_ptr<T>>: std::false_type
    {
        using type = std::unique_ptr<const T>;
    };

    template <typename T>
    struct edge_validate<std::shared_ptr<T>>: std::false_type
    {
        using type = std::shared_ptr<const T>;
    };

    template <typename T>
    struct edge_validate<std::unique_ptr<const T>>: std::true_type
    {
        using type = std::unique_ptr<const T>;
    };

    template <typename T>
    struct edge_validate<std::shared_ptr<const T>>: std::true_type
    {
        using type = std::shared_ptr<const T>;
    };

    static_assert(edge_validate<bool>::value);
    static_assert(edge_validate<std::string>::value);
    static_assert(!edge_validate<std::string&>::value);
    static_assert(!edge_validate<std::string*>::value);
    static_assert(!edge_validate<const std::string>::value);
    static_assert(!edge_validate<const std::string&>::value);
    static_assert(!edge_validate<std::unique_ptr<bool>>::value);
    static_assert(edge_validate<std::unique_ptr<const bool>>::value);
    static_assert(!edge_validate<std::shared_ptr<bool>>::value);
    static_assert(edge_validate<std::shared_ptr<const bool>>::value);
}
