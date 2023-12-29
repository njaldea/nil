#pragma once

#include <cstdint>

#include <queue>

struct IDs
{
    std::uint64_t reserve()
    {
        if (!reuse_queue.empty())
        {
            const auto v = reuse_queue.front();
            reuse_queue.pop();
            return v;
        }
        return ++current;
    }

    void release(std::uint64_t id)
    {
        reuse_queue.push(id);
    }

    std::uint64_t current = 0;
    std::queue<std::uint64_t> reuse_queue;
};
