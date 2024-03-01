#pragma once

#include "DataEdge.hpp"
#include "ICallable.hpp"

namespace nil::gate::detail
{
    template <typename T>
    class BatchEdge final: public MutableEdge<T>
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

        void set_value(T new_data) override
        {
#ifdef NIL_GATE_CHECKS
            if (!tasks)
            {
                return;
            }
#endif
            tasks->push_back(make_callable(
                [this, new_data = std::move(new_data)]()
                {
                    if (edge->exec(std::move(new_data)))
                    {
                        edge->pend();
                    }
                }
            ));
        }

        DataEdge<T>* edge = nullptr;
        std::vector<std::unique_ptr<ICallable>>* tasks = nullptr;
    };
}
