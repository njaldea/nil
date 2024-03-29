#pragma once

#include "ICallable.hpp"
#include "INode.hpp"

#include <vector>

namespace nil::gate
{
    struct IRunner
    {
        IRunner() = default;
        virtual ~IRunner() noexcept = default;

        IRunner(IRunner&&) noexcept = default;
        IRunner& operator=(IRunner&&) noexcept = default;

        IRunner(const IRunner&) = default;
        IRunner& operator=(const IRunner&) = default;

        virtual void flush(std::vector<std::unique_ptr<ICallable<void()>>> diffs) = 0;
        virtual void run(const std::vector<std::unique_ptr<INode>>& nodes) = 0;
    };
}
