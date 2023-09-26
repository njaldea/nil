#pragma once

#include <array>

namespace
{
    constexpr auto TCP_HEADER_SIZE = sizeof(std::uint64_t);

    constexpr auto UDP_REQUIRED_SIZE = 5u;
    constexpr auto UDP_INTERNAL_MESSAGE = 1u;
    constexpr auto UDP_EXTERNAL_MESSAGE = 0u;

    constexpr auto TO_BITS = 8u;
    constexpr auto START_INDEX = 0u;
}

namespace nil::service::utils
{
    std::array<std::uint8_t, sizeof(std::uint64_t)> to_array(std::uint64_t value);
    std::array<std::uint8_t, sizeof(std::uint32_t)> to_array(std::uint32_t value);
    std::array<std::uint8_t, sizeof(std::uint8_t)> to_array(std::uint8_t value);

    template <typename T>
    T from_array(const void* data)
    {
        if constexpr (sizeof(T) == 1)
        {
            return T(static_cast<const char*>(data)[0]);
        }
        else
        {
            T t = 0;
            for (auto i = 0u; i < sizeof(T); ++i)
            {
                t |= T(static_cast<const char*>(data)[i]) << (i * TO_BITS);
            }
            return t;
        }
    }
}
