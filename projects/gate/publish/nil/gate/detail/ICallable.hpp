#pragma once

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

}
