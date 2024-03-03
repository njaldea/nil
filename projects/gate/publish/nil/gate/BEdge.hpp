#pragma once

#include "detail/DataEdge.hpp"
#include "detail/ICallable.hpp"

namespace nil::gate
{
    template <typename T>
    class BatchEdge final
    {
    public:
        BatchEdge() = default;
        ~BatchEdge() noexcept = default;

        BatchEdge(BatchEdge&&) = delete;
        BatchEdge(const BatchEdge&) = delete;
        BatchEdge& operator=(BatchEdge&&) = delete;
        BatchEdge& operator=(const BatchEdge&) = delete;

        const T& value() const
        {
            return edge->value();
        }

        void set_value(T new_data)
        {
#ifdef NIL_GATE_CHECKS
            if (!tasks)
            {
                return;
            }
#endif
            tasks->push_back(detail::make_callable(
                [this, new_data = std::move(new_data)]()
                {
                    if (edge->exec(std::move(new_data)))
                    {
                        edge->pend();
                    }
                }
            ));
        }

        detail::DataEdge<T>* edge = nullptr;
        std::vector<std::unique_ptr<detail::ICallable<void()>>>* tasks = nullptr;
    };
}
