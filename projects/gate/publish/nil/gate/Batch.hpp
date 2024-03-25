#pragma once

#include "detail/DataEdge.hpp"
#include "detail/ICallable.hpp"
#include "detail/Tasks.hpp"
#include "edges/Batch.hpp"

#include <tuple>

namespace nil::gate
{
    class Core;

    template <typename... T>
    class Batch final
    {
    public:
        Batch(
            const Core* init_core,
            detail::Tasks* init_tasks,
            detail::ICallable<void(const Core*)>* init_commit,
            const std::tuple<edges::Mutable<T>*...>& init_edges
        )
            : Batch(
                  init_core,
                  init_tasks,
                  init_commit,
                  init_edges,
                  std::make_index_sequence<sizeof...(T)>()
              )
        {
        }

        ~Batch() noexcept
        {
            if (nullptr != tasks)
            {
                tasks->push_batch(std::move(batch_tasks));
            }
            if (nullptr != commit)
            {
                commit->call(core);
            }
        }

        Batch(Batch&&) = delete;
        Batch(const Batch&) = delete;
        Batch& operator=(Batch&&) = delete;
        Batch& operator=(const Batch&) = delete;

        template <std::size_t index>
        auto* get()
        {
            return std::addressof(std::get<index>(edges));
        }

    private:
        template <std::size_t... I>
        Batch(
            const Core* init_core,
            detail::Tasks* init_tasks,
            detail::ICallable<void(const Core*)>* init_commit,
            const std::tuple<edges::Mutable<T>*...>& init_edges,
            std::index_sequence<I...> /* unused */
        )
            : core(init_core)
            , tasks(init_tasks)
            , commit(init_commit)
            , edges()
        {
            (..., (initialize_edge(std::get<I>(edges), std::get<I>(init_edges))));
        }

        template <typename U>
        void initialize_edge(edges::Batch<U>& e, edges::Mutable<U>* data_edge)
        {
            e.edge = static_cast<detail::edges::Data<U>*>(data_edge);
            e.tasks = &batch_tasks;
        }

        std::vector<std::unique_ptr<detail::ICallable<void()>>> batch_tasks;
        const Core* core = nullptr;
        detail::Tasks* tasks = nullptr;
        detail::ICallable<void(const Core*)>* commit = nullptr;
        std::tuple<edges::Batch<T>...> edges;
    };

    template <std::size_t index, typename... T>
    auto get(Batch<T...>& batch)
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
    using type = decltype(get<I>(std::declval<nil::gate::Batch<T...>&>()));
};
