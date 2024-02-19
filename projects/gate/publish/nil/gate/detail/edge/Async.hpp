#pragma once

#include "../Tasks.hpp"
#include "Batch.hpp"
#include "Internal.hpp"

namespace nil::gate::detail
{
    template <typename... T>
    struct AsyncEdges final
    {
    public:
        AsyncEdges(Tasks* init_tasks, ICallable* init_commit, InternalEdge<T>*... init_edges)
            : tasks(init_tasks)
            , commit_cb(init_commit)
            , edges(init_edges...)
        {
        }

        ~AsyncEdges() noexcept = default;

        AsyncEdges(const AsyncEdges&) = default;
        AsyncEdges(AsyncEdges&&) = delete;

        AsyncEdges& operator=(const AsyncEdges&) = default;
        AsyncEdges& operator=(AsyncEdges&&) = delete;

        template <std::size_t index>
        auto get() const
        {
            return std::get<index>(edges);
        }

        Batch<T...> batch()
        {
            return Batch<T...>(tasks, commit_cb, edges);
        }

    private:
        Tasks* tasks;
        ICallable* commit_cb;
        std::tuple<InternalEdge<T>*...> edges;
    };

    template <typename... T>
    using async_edges = AsyncEdges<T...>;

    template <std::size_t index, typename... T>
    auto get(const AsyncEdges<T...>& edges)
    {
        return edges.template get<index>();
    }
}

namespace nil::gate
{
    template <typename... T>
    using async_edges = detail::AsyncEdges<T...>;
}

template <typename... T>
struct std::tuple_size<nil::gate::detail::AsyncEdges<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, typename... T>
struct std::tuple_element<I, nil::gate::detail::AsyncEdges<T...>>
{
    using type = decltype(get<I>(std::declval<nil::gate::detail::AsyncEdges<T...>>()));
};
