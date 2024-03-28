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

        Tasks(Tasks&&) noexcept = delete;
        Tasks& operator=(Tasks&&) noexcept = delete;

        Tasks(const Tasks&) = delete;
        Tasks& operator=(const Tasks&) = delete;

        void push_batch(std::vector<std::unique_ptr<ICallable<void()>>> cbs)
        {
            std::lock_guard g(mutex);
            for (auto&& cb : cbs)
            {
                tasks.push_back(std::move(cb));
            }
        }

        void push(std::unique_ptr<ICallable<void()>> cb)
        {
            std::lock_guard g(mutex);
            tasks.push_back(std::move(cb));
        }

        std::vector<std::unique_ptr<ICallable<void()>>> flush()
        {
            std::lock_guard g(mutex);
            return std::exchange(tasks, {});
        }

    private:
        std::vector<std::unique_ptr<ICallable<void()>>> tasks;
        std::mutex mutex;
    };
}
