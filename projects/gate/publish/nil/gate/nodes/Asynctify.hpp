// #pragma once

// #include "../detail/traits/node.hpp"
// #include "../types.hpp"
// #include <utility>

// namespace nil::gate::nodes
// {
//     template <
//         typename T,
//         typename Inputs = detail::traits::node<T>::inputs::type,
//         typename SyncOutput = detail::traits::node<T>::sync_outputs::type,
//         typename ASyncOutput = detail::traits::node<T>::async_outputs::type>
//     struct Asynctify;

//     template <typename T, typename... Inputs, typename... SyncOutputs, typename... AsyncOutputs>
//     struct Asynctify<
//         T,
//         nil::gate::detail::traits::types<Inputs...>,
//         nil::gate::detail::traits::types<SyncOutputs...>,
//         nil::gate::detail::traits::types<AsyncOutputs...>>
//     {
//         static_assert(sizeof...(SyncOutputs) > 0, "Node has no sync output");

//         template <typename... Args>
//         explicit Asynctify(Args&&... args)
//             : node(std::forward<Args>(args)...)
//         {
//         }

//         void operator()(
//             const nil::gate::async_outputs<SyncOutputs..., AsyncOutputs...>& asyncs,
//             Inputs&... args
//         ) const
//         {
//             using return_t = decltype(std::declval<T>.operator()(args...));
//             if constexpr (std::is_same_v<std::tuple<SyncOutputs...>, return_t>)
//             {
//                 spread(
//                     asyncs,
//                     node.operator()(args...),
//                     std::make_index_sequence<sizeof...(SyncOutputs)>()
//                 );
//             }
//             else if constexpr (std::is_same_v<void, return_t>)
//             {
//                 node.operator()(args...);
//             }
//             else
//             {
//                 get<0>(asyncs)->set_value(node.operator()(args...));
//             }
//         }

//         template <std::size_t... o_indices>
//         void spread(
//             const nil::gate::async_outputs<SyncOutputs..., AsyncOutputs...>& asyncs,
//             std::tuple<SyncOutputs...> output,
//             std::index_sequence<o_indices...> /* unused */
//         )
//         {
//             (..., get<o_indices>(asyncs)->set_value(get<o_indices>(output)));
//         }

//         T node;
//     };
// }
