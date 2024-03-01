#pragma once

#include "MEdge.hpp"
#include "REdge.hpp"

#include <tuple>

namespace nil::gate
{
    /**
     *  namespace ng = nil::gate;
     *  std::tuple<O...> Node::operator()(I...)
     *      ng::inputs<I...>
     *      ng::outputs<O...>
     *      ng::sync_outputs<O...>
     *      ng::async_outputs<>
     *
     *  std::tuple<O...> Node::operator()(ng::async_outputs<A...>, I...)
     *      ng::inputs<I...>
     *      ng::outputs<O..., A...>
     *      ng::sync_outputs<O...>
     *      ng::async_outputs<A...>
     *
     *  std::tuple<O...> Node::operator()(ng::async_outputs<A...>, const ng::Core&, I...)
     *      ng::inputs<I...>
     *      ng::outputs<O..., A...>
     *      ng::sync_outputs<O...>
     *      ng::async_outputs<A...>
     */

    template <typename... T>
    using inputs = std::tuple<ReadOnlyEdge<T>*...>;
    template <typename... T>
    using sync_outputs = std::tuple<ReadOnlyEdge<T>*...>;
    template <typename... T>
    using async_outputs = std::tuple<MutableEdge<T>*...>;
    template <typename... T>
    using outputs = std::tuple<ReadOnlyEdge<T>*...>;
}
