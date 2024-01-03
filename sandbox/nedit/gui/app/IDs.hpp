#pragma once

#include <cstdint>

#include <stack>

struct IDs
{
    std::uint64_t reserve()
    {
        if (reuse_ids.empty())
        {
            return ++current;
        }
        const auto v = reuse_ids.top();
        reuse_ids.pop();
        return v;
    }

    void release(std::uint64_t id)
    {
        reuse_ids.push(id);
    }

    std::uint64_t current = 0;
    std::stack<std::uint64_t> reuse_ids;
};
