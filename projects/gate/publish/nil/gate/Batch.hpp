#pragma once

#include "Diffs.hpp"
#include "ICallable.hpp"

#include "detail/DataEdge.hpp"
#include "edges/Batch.hpp"

#include <tuple>

#ifdef NIL_GATE_CHECKS
#include <cassert>
#endif

namespace nil::gate
{
    class Core;

    template <typename... T>
    class Batch final
    {
    public:
        Batch(
            const Core* init_core,
            Diffs* init_diffs,
            ICallable<void(const Core*)>* init_commit,
            const std::tuple<edges::Mutable<T>*...>& init_edges
        )
            : Batch(
                  init_core,
                  init_diffs,
                  init_commit,
                  init_edges,
                  std::make_index_sequence<sizeof...(T)>()
              )
        {
        }

        ~Batch() noexcept
        {
#ifdef NIL_GATE_CHECKS
            assert(nullptr != diffs);
#endif
            diffs->push_batch(std::move(batch_diffs));
            if (nullptr != commit)
            {
                commit->call(core);
            }
        }

        Batch(Batch&&) noexcept = delete;
        Batch& operator=(Batch&&) noexcept = delete;

        Batch(const Batch&) = delete;
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
            Diffs* init_diffs,
            ICallable<void(const Core*)>* init_commit,
            const std::tuple<edges::Mutable<T>*...>& init_edges,
            std::index_sequence<I...> /* unused */
        )
            : core(init_core)
            , diffs(init_diffs)
            , commit(init_commit)
            , edges()
        {
            (..., (initialize_edge(std::get<I>(edges), std::get<I>(init_edges))));
        }

        template <typename U>
        void initialize_edge(edges::Batch<U>& e, edges::Mutable<U>* data_edge)
        {
            e.edge = static_cast<detail::edges::Data<U>*>(data_edge);
            e.diffs = &batch_diffs;

#ifdef NIL_GATE_CHECKS
            assert(e.edge->validate(diffs));
#endif
        }

        std::vector<std::unique_ptr<ICallable<void()>>> batch_diffs;
        const Core* core = nullptr;
        Diffs* diffs = nullptr;
        ICallable<void(const Core*)>* commit = nullptr;
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
