#pragma once

#include <memory>

namespace nil::gate::detail
{
    struct ICallable
    {
        ICallable() = default;
        virtual ~ICallable() noexcept = default;

        ICallable(ICallable&&) = delete;
        ICallable(const ICallable&) = delete;
        ICallable& operator=(ICallable&&) = delete;
        ICallable& operator=(const ICallable&) = delete;

        virtual void call() = 0;
    };

    template <typename CB>
    std::unique_ptr<ICallable> make_callable(CB&& cb)
    {
        struct Callable: ICallable
        {
            Callable(CB&& init_cb)
                : cb(std::forward<CB>(init_cb))
            {
            }

            void call() override
            {
                cb();
            }

            CB cb;
        };

        return std::make_unique<Callable>(std::forward<CB>(cb));
    }
}
