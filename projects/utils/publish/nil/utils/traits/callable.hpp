#pragma once

#include <tuple>
#include <utility>

namespace nil::utils::traits
{
    template <typename... T>
    struct types
    {
    };

    template <typename T>
    struct callable: callable<decltype(&T::operator())>
    {
    };

    template <typename T, typename R, typename... Args>
    struct callable<R (T::*)(Args...) const>: callable<R(Args...)>
    {
    };

    template <typename T, typename R, typename... Args>
    struct callable<R (T::*)(Args...)>: callable<R(Args...)>
    {
    };

    template <typename... I, typename O>
    struct callable<O(I...)>: callable<types<O>(I...)>
    {
    };

    template <typename... I>
    struct callable<void(I...)>: callable<types<>(I...)>
    {
    };

    template <typename... I, typename... O>
    struct callable<types<O...>(I...)>
    {
        using type = nil::utils::traits::types<O...>(I...);
        using inputs = types<I...>;
        using outputs = types<O...>;
    };

    template <typename... I, typename... O>
    struct callable<std::tuple<O...>(I...)>: callable<types<O...>(I...)>
    {
    };
}
