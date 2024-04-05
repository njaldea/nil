#pragma once

#include <memory>

namespace nil::gate
{
    template <typename CB>
    struct ICallable;

    template <typename R, typename... A>
    struct ICallable<R(A...)>
    {
        ICallable() = default;
        virtual ~ICallable() noexcept = default;

        ICallable(ICallable&&) noexcept = delete;
        ICallable& operator=(ICallable&&) noexcept = delete;

        ICallable(const ICallable&) = delete;
        ICallable& operator=(const ICallable&) = delete;

        virtual R call(A...) = 0;
    };

    template <typename CB, typename... Args>
    std::unique_ptr<ICallable<void(Args...)>> make_callable(CB&& cb)
    {
        struct Callable: ICallable<void(Args...)>
        {
            explicit Callable(CB init_cb)
                : cb(std::move(init_cb))
            {
            }

            ~Callable() noexcept override = default;

            Callable(Callable&&) noexcept = delete;
            Callable& operator=(Callable&&) noexcept = delete;

            Callable(const Callable&) = delete;
            Callable& operator=(const Callable&) = delete;

            void call(Args... args) override
            {
                cb(args...);
            }

            CB cb;
        };

        return std::make_unique<Callable>(std::forward<CB>(cb));
    }
}
