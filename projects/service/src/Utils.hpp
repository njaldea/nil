#pragma once

#include <boost/asio/ip/tcp.hpp>

#include <array>

namespace nil::service::utils
{
    inline std::string to_string(const boost::asio::ip::tcp::endpoint& endpoint)
    {
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    // in tcp, we need to know the size of the actual message
    // to know until when to stop
    constexpr auto TCP_HEADER_SIZE = sizeof(std::uint64_t);

    // in udp, there is no connection guarantee.
    // this flag
    constexpr std::uint8_t UDP_INTERNAL_MESSAGE = 1u;
    constexpr std::uint8_t UDP_EXTERNAL_MESSAGE = 0u;

    constexpr auto TO_BITS = 8u;
    constexpr auto START_INDEX = 0u;

    template <typename T>
    std::array<std::uint8_t, sizeof(T)> to_array(T value)
    {
        if constexpr (sizeof(T) == 1)
        {
            return {value};
        }
        else
        {
            std::array<std::uint8_t, sizeof(T)> retval;
            for (auto i = 0u; i < sizeof(T); ++i)
            {
                retval[i] = std::uint8_t(value >> (i * TO_BITS));
            }
            return retval;
        }
    }

    template <typename T>
    T from_array(const std::uint8_t* data)
    {
        if constexpr (sizeof(T) == 1)
        {
            return T(data[0]);
        }
        else
        {
            auto t = T();
            for (auto i = 0u; i < sizeof(T); ++i)
            {
                t |= T(data[i]) << (i * TO_BITS);
            }
            return t;
        }
    }
}
