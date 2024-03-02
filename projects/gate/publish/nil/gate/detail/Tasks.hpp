#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "ICallable.hpp"

namespace nil::gate::detail
{
    class Tasks final
    {
    public:
        Tasks() = default;
        ~Tasks() noexcept = default;

        Tasks(Tasks&&) = default;
        Tasks(const Tasks&) = default;
        Tasks& operator=(Tasks&&) = default;
        Tasks& operator=(const Tasks&) = default;

        void push_batch(std::vector<std::unique_ptr<ICallable>> cbs)
        {
            std::lock_guard g(mutex);
            for (auto&& cb : cbs)
            {
                tasks.push_back(std::move(cb));
            }
        }

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
