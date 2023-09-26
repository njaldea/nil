#include "Utils.hpp"

namespace nil::service::utils
{
    std::array<std::uint8_t, 1u> to_array(std::uint8_t value)
    {
        return {value};
    }

    std::array<std::uint8_t, sizeof(std::uint32_t)> to_array(std::uint32_t value)
    {
        return {
            std::uint8_t(value >> (0 * TO_BITS)), //
            std::uint8_t(value >> (1 * TO_BITS)), //
            std::uint8_t(value >> (2 * TO_BITS)), //
            std::uint8_t(value >> (3 * TO_BITS))  //
        };
    }

    std::array<std::uint8_t, sizeof(std::uint64_t)> to_array(std::uint64_t value)
    {
        return {
            std::uint8_t(value >> (0 * TO_BITS)), //
            std::uint8_t(value >> (1 * TO_BITS)), //
            std::uint8_t(value >> (2 * TO_BITS)), //
            std::uint8_t(value >> (3 * TO_BITS)), //
            std::uint8_t(value >> (4 * TO_BITS)), //
            std::uint8_t(value >> (5 * TO_BITS)), //
            std::uint8_t(value >> (6 * TO_BITS)), //
            std::uint8_t(value >> (7 * TO_BITS))  //
        };
    }
}
