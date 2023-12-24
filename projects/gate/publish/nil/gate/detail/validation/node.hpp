#pragma once

#include <memory>
#include <type_traits>

namespace nil::gate::detail
{
    template <typename T>
    struct node_validate
        : std::conditional_t<
              std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>,
              std::true_type,
              std::false_type>
    {
    };

    template <typename T>
    struct node_validate<T*>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<T&>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<const T&>: node_validate<T>
    {
    };

    template <typename T>
    struct node_validate<const std::unique_ptr<T>&>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<std::shared_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<std::shared_ptr<const T>>: std::true_type
    {
    };

    template <typename T>
    struct node_validate<const std::shared_ptr<T>&>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<const std::unique_ptr<const T>&>: std::true_type
    {
    };

    template <typename T>
    struct node_validate<const std::shared_ptr<const T>&>: std::true_type
    {
    };

    template <typename T>
    struct node_validate_i: node_validate<T>
    {
    };

    template <typename T>
    struct node_validate_o: node_validate<T>
    {
    };

    template <typename T>
    struct node_validate_o<T&>: std::false_type
    {
    };

    static_assert(node_validate<bool>::value);
    static_assert(node_validate<std::string>::value);
    static_assert(node_validate<const std::string>::value);
    static_assert(node_validate<const std::string&>::value);
    static_assert(!node_validate<std::string&>::value);
    static_assert(!node_validate<std::string*>::value);

    static_assert(!node_validate<std::unique_ptr<bool>>::value);
    static_assert(!node_validate<std::shared_ptr<bool>>::value);

    static_assert(!node_validate<std::unique_ptr<const bool>>::value);
    static_assert(node_validate<std::shared_ptr<const bool>>::value);

    static_assert(!node_validate<const std::unique_ptr<bool>&>::value);
    static_assert(!node_validate<const std::shared_ptr<bool>&>::value);

    static_assert(node_validate<const std::unique_ptr<const bool>&>::value);
    static_assert(node_validate<const std::shared_ptr<const bool>&>::value);
}
