#pragma once

#include "../detail/DataEdge.hpp"
#include "../detail/ICallable.hpp"

namespace nil::gate::edges
{
    template <typename T>
    class Batch final
    {
    public:
        Batch() = default;
        ~Batch() noexcept = default;

        Batch(Batch&&) = delete;
        Batch(const Batch&) = delete;
        Batch& operator=(Batch&&) = delete;
        Batch& operator=(const Batch&) = delete;

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
                [e = edge, d = std::move(new_data)]() mutable
                {
                    if (e->exec(d))
                    {
                        e->pend();
                    }
                }
            ));
        }

        detail::edges::Data<T>* edge = nullptr;
        std::vector<std::unique_ptr<detail::ICallable<void()>>>* tasks = nullptr;
    };
}
