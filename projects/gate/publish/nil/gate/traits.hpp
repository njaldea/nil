#pragma once

#include "detail/callable_traits.hpp"

namespace nil::gate
{
    //  TODO: figure out the minimum type aliases needed
    //  for users to wrap nil/gate. best use case will be
    //  from the result of nedit explorations.
    template <typename T>
    struct node_traits
    {
        using inputs = detail::callable_traits<T>::inputs::type;
        using outputs = detail::callable_traits<T>::outputs::type;
    };
}
