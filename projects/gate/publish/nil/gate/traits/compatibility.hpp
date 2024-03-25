#pragma once

namespace nil::gate::traits
{
    template <typename T, typename U>
    struct compatibility;

    template <typename T>
    struct compatibility<T, T>
    {
        static const T& convert(const T& u)
        {
            return u;
        }
    };
}
