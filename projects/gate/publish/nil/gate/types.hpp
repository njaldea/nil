#pragma once

#include "edges/Compatible.hpp"
#include "edges/Mutable.hpp"
#include "edges/ReadOnly.hpp"

#include <tuple>

namespace nil::gate
{
    template <typename... T>
    using inputs = std::tuple<edges::Compatible<T>...>;
    template <typename... T>
    using sync_outputs = std::tuple<edges::ReadOnly<T>*...>;
    template <typename... T>
    using async_outputs = std::tuple<edges::Mutable<T>*...>;
    template <typename... T>
    using outputs = std::tuple<edges::ReadOnly<T>*...>;
}
