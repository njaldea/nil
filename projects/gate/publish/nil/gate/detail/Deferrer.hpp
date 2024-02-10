#pragma once

#include <memory>
#include <mutex>
#include <vector>

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

    struct Deferrer
    {
        std::vector<std::unique_ptr<ICallable>> tasks;
        std::mutex mutex;
    };
}
