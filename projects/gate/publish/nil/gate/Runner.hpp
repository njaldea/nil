#pragma once

#include "IRunner.hpp"

namespace nil::gate
{
    class Runner final: public IRunner
    {
    public:
        void flush(std::vector<std::unique_ptr<ICallable<void()>>> diffs) override
        {
            for (const auto& d : diffs)
            {
                d->call();
            }
        }

        void run(const std::vector<std::unique_ptr<INode>>& nodes) override
        {
            for (const auto& node : nodes)
            {
                if (node->is_pending() && node->is_ready())
                {
                    node->exec();
                    node->done();
                }
            }
        }
    };
}
