#pragma once

#include <cstdint>

namespace nil::gatex
{
    template <typename T>
    class identity
    {
    private:
        static constexpr const std::uint8_t identifier = 0;

    public:
        static constexpr auto value = static_cast<const void*>(&identifier);
    };

    template <typename T>
    constexpr auto identity_v = identity<T>::value;
}
