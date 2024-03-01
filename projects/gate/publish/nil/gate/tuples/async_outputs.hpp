#pragma once

#include "../MEdge.hpp"

#include "edges.hpp"

namespace nil::gate
{
    template <typename... T>
    using async_outputs = edges<MutableEdge, T...>;
}
