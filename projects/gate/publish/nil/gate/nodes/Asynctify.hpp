#pragma once

#include "../Async.hpp"
#include "../detail/ICallable.hpp"
#include "../detail/traits/node.hpp"

namespace nil::gate::nodes
{
    template <
        typename T,
        typename Inputs = detail::traits::node<T>::inputs::type,
        typename SyncOutput = detail::traits::node<T>::sync_outputs::type,
        typename ASyncOutput = detail::traits::node<T>::async_outputs::type>
    struct Asynctify;

    template <typename T, typename... Inputs, typename... SyncOutputs, typename... AsyncOutputs>
    struct Asynctify<
        T,
        nil::gate::detail::traits::types<Inputs...>,
        nil::gate::detail::traits::types<SyncOutputs...>,
        nil::gate::detail::traits::types<AsyncOutputs...>>
    {
        static_assert(sizeof...(SyncOutputs) > 0, "Node has no sync output");

        template <typename... Args>
        Asynctify(Args&&... args)
            : node(std::forward<Args>(args)...)
        {
        }

        auto operator()(
            const nil::gate::async_edges<SyncOutputs..., AsyncOutputs...>& asyncs,
            Inputs&... args
        ) const
        {
            return node.operator()(args...);
        }

        T node;
    };
}
