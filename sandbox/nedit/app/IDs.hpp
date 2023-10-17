#pragma once

#include <cstdint>

struct IDs
{
    std::uint64_t reserve()
    {
        return ++current;
    }

    std::uint64_t current = 0;
};
