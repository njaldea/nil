#pragma once

#include "types.hpp"

#include <tuple>

namespace nil::gate::detail::traits
{
    enum class EReturnType
    {
        Void,
        Tuple,
        Mono
    };

    template <typename T>
    struct callable: callable<decltype(&T::operator())>
    {
    };

    template <typename R, typename... Args>
    struct callable<R (*)(Args...)>: callable<R(Args...)>
    {
    };

    template <typename T, typename R, typename... Args>
    struct callable<R (T::*)(Args...) const>: callable<R(Args...)>
    {
    };

    // TODO:
    //  re-evaluate if I should allow non-const operator().
    //  ideally, nodes should be stateless so having a non-const operator()
    //  should not be necessary.
    //
    // template <typename T, typename R, typename... Args>
    // struct callable<R (T::*)(Args...)>: callable<R(Args...)>
    // {
    // };

    template <typename... I, typename... O>
    struct callable<std::tuple<O...>(I...)>: callable<types<O...>(I...)>
    {
        static constexpr auto tag = EReturnType::Tuple;
    };

    template <typename... I>
    struct callable<void(I...)>: callable<types<>(I...)>
    {
        static constexpr auto tag = EReturnType::Void;
    };

    template <typename... I, typename O>
    struct callable<O(I...)>: callable<types<O>(I...)>
    {
        static constexpr auto tag = EReturnType::Mono;
    };

    template <typename... I, typename... O>
    struct callable<types<O...>(I...)>
    {
        using type = types<O...>(I...);
        using inputs = types<I...>;
        using outputs = types<O...>;
    };
}
