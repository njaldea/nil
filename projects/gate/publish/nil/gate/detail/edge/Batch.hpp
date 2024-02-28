#pragma once

#include "Internal.hpp"

#include "../ICallable.hpp"

namespace nil::gate::detail
{
    template <typename T>
    class BatchEdge final: public InternalEdge<T>
    {
    public:
        BatchEdge() = default;
        ~BatchEdge() noexcept = default;

        BatchEdge(BatchEdge&&) = delete;
        BatchEdge(const BatchEdge&) = delete;

        BatchEdge& operator=(BatchEdge&&) = delete;
        BatchEdge& operator=(const BatchEdge&) = delete;

        const T& value() const override
        {
            return edge->value();
        }

        bool has_value() const override
        {
            return edge->has_value();
        }

        void set_value(T new_data) override
        {
            if (tasks)
            {
                struct Callable: ICallable
                {
                    Callable(T init_data, InternalEdge<T>* init_parent)
                        : data(std::move(init_data))
                        , parent(init_parent)
                    {
                    }

                    void call() override
                    {
                        if (parent->exec(std::move(data)))
                        {
                            parent->pend();
                        }
                    }

                    T data;
                    InternalEdge<T>* parent;
                };

                tasks->push_back(std::make_unique<Callable>(std::move(new_data), this));
            }
        }

        bool exec(T new_data) override
        {
            return edge->exec(std::move(new_data));
        }

        void pend() override
        {
            edge->pend();
        }

        void attach_output(INode* node) override
        {
            edge->attach_output(node);
        }

        std::uint64_t depth() const override
        {
            return edge->depth();
        }

        MutableEdge<T>* as_mutable() const
        {
            return edge;
        }

        InternalEdge<T>* edge = nullptr;
        std::vector<std::unique_ptr<ICallable>>* tasks = nullptr;
    };

    template <typename... T>
    class Batch final
    {
    public:
        Batch(Tasks* init_tasks, ICallable* init_commit, std::tuple<InternalEdge<T>*...> init_edges)
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
            return std::get<index>(edges).as_mutable();
        }

    private:
        template <std::size_t... I>
        Batch(Tasks* init_tasks, ICallable* init_commit, std::tuple<InternalEdge<T>*...> init_edges, std::index_sequence<I...>)
            : tasks(init_tasks)
            , commit(init_commit)
            , edges()
        {
            (..., (std::get<I>(edges).edge = std::get<I>(init_edges)));
            (..., (std::get<I>(edges).tasks = &batch_tasks));
        }

        std::vector<std::unique_ptr<ICallable>> batch_tasks;
        Tasks* tasks;
        ICallable* commit;
        std::tuple<BatchEdge<T>...> edges;
    };

    template <std::size_t index, typename... T>
    auto get(const Batch<T...>& batch)
    {
        return batch.template get<index>();
    }
}

namespace nil::gate
{
    template <typename... T>
    using Batch = detail::Batch<T...>;
}

template <typename... T>
struct std::tuple_size<nil::gate::detail::Batch<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, typename... T>
struct std::tuple_element<I, nil::gate::detail::Batch<T...>>
{
    using type = decltype(get<I>(std::declval<nil::gate::detail::Batch<T...>>()));
};
