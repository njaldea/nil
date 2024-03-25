#pragma once

#include "../traits/compatibility.hpp"

#include <functional>

namespace nil::gate::traits
{
    template <typename T>
    struct compatibility<T, std::reference_wrapper<const T>>
    {
        static const T& convert(const std::reference_wrapper<const T>& u)
        {
            return u;
        }
    };

    template <typename T>
    struct compatibility<std::reference_wrapper<const T>, T>
    {
        static std::reference_wrapper<const T> convert(const T& u)
        {
            return std::cref(u);
        }
    };
}
