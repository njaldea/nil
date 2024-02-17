#pragma once

#include "detail/types.hpp"

namespace nil::gate
{
    //  TODO: figure out the minimum type aliases needed
    //  for users to wrap nil/gate. best use case will be
    //  from the result of nedit explorations.
    template <typename T>
    struct node_traits
    {
        using inputs = detail::traits<T>::i::type;
        using outputs = detail::traits<T>::outs::type;
    };
}
