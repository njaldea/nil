#pragma once

#include <memory>

namespace nil::gate::detail
{
    struct ICallable
    {
        ICallable() = default;
        virtual ~ICallable() = default;

        ICallable(ICallable&&) = delete;
        ICallable(const ICallable&) = delete;
        ICallable& operator=(ICallable&&) = delete;
        ICallable& operator=(const ICallable&) = delete;

        virtual void call() = 0;
    };

    template <typename CB>
    auto make_callable(CB cb)
    {
        struct Callable: ICallable
        {
            Callable(CB init_cb)
                : cb(std::move(init_cb))
            {
            }

            void call() override
            {
                cb();
            }

            CB cb;
        };

        return std::make_unique<Callable>(std::move(cb));
    }
}
