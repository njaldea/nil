#pragma once

namespace nil::gate::traits
{
    template <typename T>
    struct edgify
    {
        using type = T;
    };

    template <typename T>
    using edgify_t = typename edgify<T>::type;
}
