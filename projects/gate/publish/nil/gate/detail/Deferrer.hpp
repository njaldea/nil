#pragma once

#include <memory>
#include <mutex>
#include <utility>
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

    class Deferrer
    {
    public:
        void push(std::unique_ptr<ICallable> cb)
        {
            std::lock_guard g(mutex);
            tasks.push_back(std::move(cb));
        }

        std::vector<std::unique_ptr<ICallable>> flush()
        {
            std::lock_guard g(mutex);
            return std::exchange(tasks, {});
        }

    private:
        std::vector<std::unique_ptr<ICallable>> tasks;
        std::mutex mutex;
    };
}
