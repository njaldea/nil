#pragma once

#include "./detail/BatchEdge.hpp"
#include "./detail/DataEdge.hpp"

#include "detail/ICallable.hpp"
#include "detail/Tasks.hpp"

namespace nil::gate
{
    template <typename... T>
    class Batch final
    {
    public:
        Batch(
            detail::Tasks* init_tasks,
            detail::ICallable* init_commit,
            std::tuple<detail::DataEdge<T>*...> init_edges
        )
            : Batch(
                  init_tasks,
                  init_commit,
                  std::move(init_edges),
                  std::make_index_sequence<sizeof...(T)>()
              )
        {
        }

        ~Batch() noexcept
        {
            if (tasks)
            {
                tasks->push_batch(std::move(batch_tasks));
            }
            if (commit)
            {
                commit->call();
            }
        }

        Batch(const Batch&) = delete;
        Batch(Batch&&) = default;

        Batch& operator=(const Batch&) = delete;
        Batch& operator=(Batch&&) = default;

        template <std::size_t index>
        auto* get() const
        {
            return as_mutable(std::get<index>(edges).edge);
        }

    private:
        template <std::size_t... I>
        Batch(
            detail::Tasks* init_tasks,
            detail::ICallable* init_commit,
            std::tuple<detail::DataEdge<T>*...> init_edges,
            std::index_sequence<I...> //
        )
            : tasks(init_tasks)
            , commit(init_commit)
            , edges()
        {
            (..., (initialize_edge(std::get<I>(edges), std::get<I>(init_edges))));
        }

        template <typename U>
        void initialize_edge(detail::BatchEdge<U>& e, detail::DataEdge<U>* data_edge)
        {
            e.edge = data_edge;
            e.tasks = &batch_tasks;
        }

        std::vector<std::unique_ptr<detail::ICallable>> batch_tasks;
        detail::Tasks* tasks;
        detail::ICallable* commit;
        std::tuple<detail::BatchEdge<T>...> edges;
    };

    template <std::size_t index, typename... T>
    auto get(const Batch<T...>& batch)
    {
        return batch.template get<index>();
    }
}

template <typename... T>
struct std::tuple_size<nil::gate::Batch<T...>>: std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, typename... T>
struct std::tuple_element<I, nil::gate::Batch<T...>>
{
    using type = decltype(get<I>(std::declval<nil::gate::Batch<T...>>()));
};
