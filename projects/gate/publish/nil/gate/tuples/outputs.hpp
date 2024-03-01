#pragma once

#include "../REdge.hpp"

#include "edges.hpp"

namespace nil::gate
{
    template <typename... T>
    using outputs = edges<ReadOnlyEdge, T...>;
}
