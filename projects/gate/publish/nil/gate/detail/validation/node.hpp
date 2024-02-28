#pragma once

#include <memory>
#include <optional>
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
    struct node_validate<std::unique_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<std::shared_ptr<T>>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<std::optional<T>>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<std::shared_ptr<const T>>: std::true_type
    {
    };

    template <typename T>
    struct node_validate<std::optional<const T>>: std::true_type
    {
    };

    template <typename T>
    struct node_validate<const std::unique_ptr<T>&>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<const std::shared_ptr<T>&>: std::false_type
    {
    };

    template <typename T>
    struct node_validate<const std::optional<T>&>: std::false_type
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
    struct node_validate<const std::optional<const T>&>: std::true_type
    {
    };

    template <typename T>
    struct node_validate_i final: node_validate<T>
    {
    };

    static_assert(node_validate_i<bool>::value);
    static_assert(node_validate_i<const bool>::value);
    static_assert(node_validate_i<const bool&>::value);

    static_assert(!node_validate_i<bool&>::value);
    static_assert(!node_validate_i<bool*>::value);
    static_assert(!node_validate_i<const bool*>::value);

    static_assert(!node_validate_i<std::unique_ptr<bool>>::value);
    static_assert(!node_validate_i<std::shared_ptr<bool>>::value);
    static_assert(!node_validate_i<std::optional<bool>>::value);

    static_assert(!node_validate_i<std::unique_ptr<const bool>>::value);
    static_assert(node_validate_i<std::shared_ptr<const bool>>::value);
    static_assert(node_validate_i<std::optional<const bool>>::value);

    static_assert(!node_validate_i<std::unique_ptr<bool>&>::value);
    static_assert(!node_validate_i<std::shared_ptr<bool>&>::value);
    static_assert(!node_validate_i<std::optional<bool>&>::value);

    static_assert(!node_validate_i<const std::unique_ptr<bool>&>::value);
    static_assert(!node_validate_i<const std::shared_ptr<bool>&>::value);
    static_assert(!node_validate_i<const std::optional<bool>&>::value);

    static_assert(!node_validate_i<std::unique_ptr<const bool>&>::value);
    static_assert(!node_validate_i<std::shared_ptr<const bool>&>::value);
    static_assert(!node_validate_i<std::optional<const bool>&>::value);

    static_assert(node_validate_i<const std::unique_ptr<const bool>&>::value);
    static_assert(node_validate_i<const std::shared_ptr<const bool>&>::value);
    static_assert(node_validate_i<const std::optional<const bool>&>::value);

    template <typename T>
    struct node_validate_s final: node_validate<T>
    {
    };

    template <typename T>
    struct node_validate_s<const T> final: std::false_type
    {
    };

    template <typename T>
    struct node_validate_s<T&> final: std::false_type
    {
    };

    template <typename T>
    struct node_validate_s<std::unique_ptr<const T>> final: std::true_type
    {
    };

    static_assert(node_validate_s<bool>::value);
    static_assert(!node_validate_s<const bool>::value);
    static_assert(!node_validate_s<const bool&>::value);

    static_assert(!node_validate_s<bool&>::value);
    static_assert(!node_validate_s<bool*>::value);
    static_assert(!node_validate_s<const bool*>::value);

    static_assert(!node_validate_s<std::unique_ptr<bool>>::value);
    static_assert(!node_validate_s<std::shared_ptr<bool>>::value);
    static_assert(!node_validate_s<std::optional<bool>>::value);

    static_assert(node_validate_s<std::unique_ptr<const bool>>::value);
    static_assert(node_validate_s<std::shared_ptr<const bool>>::value);
    static_assert(node_validate_s<std::optional<const bool>>::value);

    static_assert(!node_validate_s<std::unique_ptr<bool>&>::value);
    static_assert(!node_validate_s<std::shared_ptr<bool>&>::value);
    static_assert(!node_validate_s<std::optional<bool>&>::value);

    static_assert(!node_validate_s<const std::unique_ptr<bool>&>::value);
    static_assert(!node_validate_s<const std::shared_ptr<bool>&>::value);
    static_assert(!node_validate_s<const std::optional<bool>&>::value);

    static_assert(!node_validate_s<std::unique_ptr<const bool>&>::value);
    static_assert(!node_validate_s<std::shared_ptr<const bool>&>::value);
    static_assert(!node_validate_s<std::optional<const bool>&>::value);

    static_assert(!node_validate_s<const std::unique_ptr<const bool>&>::value);
    static_assert(!node_validate_s<const std::shared_ptr<const bool>&>::value);
    static_assert(!node_validate_s<const std::optional<const bool>&>::value);
}
