#pragma once

#include "types.hpp"

#include <tuple>
#include <utility>

namespace nil::gate::detail::traits
{
    /**
     * @brief default/fallback type.
     *  tries to move forward by checking the singature of call operator.
     *
     * @tparam T
     */
    template <typename T>
    struct callable: callable<decltype(&T::operator())>
    {
    };

    /**
     * @brief pointer to const member method type.
     *  tries to simplify signature to plain method signature.
     *
     * @tparam T    Class Type
     * @tparam R    Return Type
     * @tparam Args Argumen Types
     */
    template <typename T, typename R, typename... Args>
    struct callable<R (T::*)(Args...) const>: callable<R(Args...)>
    {
    };

    /**
     * @brief pointer to member method type.
     *  tries to simplify signature to plain method signature.
     *
     * @tparam T    Class Type
     * @tparam R    Return Type
     * @tparam Args Argumen Types
     */

    template <typename T, typename R, typename... Args>
    struct callable<R (T::*)(Args...)>: callable<R(Args...)>
    {
    };

    /**
     * @brief specialization for `std::tuple` return.
     *  converts `std::tuple<O...>` to `types<O...>`.
     *
     * @tparam I
     * @tparam O
     */
    template <typename... I, typename... O>
    struct callable<std::tuple<O...>(I...)>: callable<types<O...>(I...)>
    {
    };

    /**
     * @brief specialized type for void return.
     *  converts `void` to an empty `types<>`.
     *
     * @tparam I
     */
    template <typename... I>
    struct callable<void(I...)>: callable<types<>(I...)>
    {
    };

    /**
     * @brief fallback type for cases that O is not a `std::tuple` or `types`.
     *  wraps `O` with `types`.
     *
     * @tparam I
     * @tparam O
     */
    template <typename... I, typename O>
    struct callable<O(I...)>: callable<types<O>(I...)>
    {
    };

    /**
     * @brief end of type recursion.
     *
     * @tparam I
     * @tparam O
     */
    template <typename... I, typename... O>
    struct callable<types<O...>(I...)>
    {
        using type = types<O...>(I...);
        using inputs = types<I...>;
        using outputs = types<O...>;
    };
}
