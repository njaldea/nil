#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "ICallable.hpp"

namespace nil::gate
{
    class Diffs final
    {
    public:
        Diffs() = default;
        ~Diffs() noexcept = default;

        Diffs(Diffs&&) noexcept = delete;
        Diffs& operator=(Diffs&&) noexcept = delete;

        Diffs(const Diffs&) = delete;
        Diffs& operator=(const Diffs&) = delete;

        void push_batch(std::vector<std::unique_ptr<ICallable<void()>>> cbs)
        {
            std::lock_guard g(mutex);
            for (auto&& cb : cbs)
            {
                diffs.push_back(std::move(cb));
            }
        }

        void push(std::unique_ptr<ICallable<void()>> cb)
        {
            std::lock_guard g(mutex);
            diffs.push_back(std::move(cb));
        }

        std::vector<std::unique_ptr<ICallable<void()>>> flush()
        {
            std::lock_guard g(mutex);
            return std::exchange(diffs, {});
        }

    private:
        std::vector<std::unique_ptr<ICallable<void()>>> diffs;
        std::mutex mutex;
    };
}
