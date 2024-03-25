#pragma once

#include <memory>
#include <optional>
#include <type_traits>

//  [TODO]
//   -  arguments should be strict (const reference, or pointer to const, ...)
//   -  return type should be relaxed
//   -  async should satisfy edge criteria
//   -  if possible, make inputs follow the following criteria
//       -  should not be reference to non-const
//       -  should not be pointer
//       -  if it has operator*, it should be a const type
//       -  if non-copy-able, it should be const ref
//       -  disallow referrence_wrapper as an input
//   -  allow reference_wrapper to const as an ouput
//   -  but disallow reference_wrapper for core.edge instantiation

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
}
