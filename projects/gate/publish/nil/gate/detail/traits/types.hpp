#pragma once

#include <tuple>

namespace nil::gate::detail::traits
{
    /**
     * @brief Blank container of types intended to be used for type deduction.
     *  an alternative to std::tuple but with zero size since this is only intended
     *  to be used for template type deduction.
     * @tparam T
     */
    template <typename... T>
    struct types
    {
        static constexpr auto size = sizeof...(T);
    };

    template <typename T>
    struct typify;

    template <template <typename> typename B, typename... T>
    struct typify<std::tuple<B<T>*...>>
    {
        using type = types<T...>;
    };

    template <typename T>
    using typify_t = typify<T>::type;
}
