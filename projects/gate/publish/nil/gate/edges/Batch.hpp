#pragma once

#include "../detail/DataEdge.hpp"
#include "../detail/ICallable.hpp"

#ifdef NIL_GATE_CHECKS
#include <cassert>
#endif

namespace nil::gate::edges
{
    template <typename T>
    class Batch final
    {
    public:
        Batch() = default;
        ~Batch() noexcept = default;

        Batch(Batch&&) noexcept = delete;
        Batch& operator=(Batch&&) noexcept = delete;

        Batch(const Batch&) = delete;
        Batch& operator=(const Batch&) = delete;

        const T& value() const
        {
#ifdef NIL_GATE_CHECKS
            assert(nullptr != edge);
#endif
            return edge->value();
        }

        void set_value(T new_data)
        {
#ifdef NIL_GATE_CHECKS
            assert(nullptr != edge);
            assert(nullptr != tasks);
#endif
            tasks->push_back(detail::make_callable(
                [e = edge, d = std::move(new_data)]() mutable
                {
                    if (e->exec(d))
                    {
                        e->pend();
                        e->done();
                    }
                }
            ));
        }

        detail::edges::Data<T>* edge = nullptr;
        std::vector<std::unique_ptr<detail::ICallable<void()>>>* tasks = nullptr;
    };
}
