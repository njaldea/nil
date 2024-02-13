#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <array>

namespace nil::service::utils
{
    inline std::string to_string(const boost::asio::ip::tcp::endpoint& endpoint)
    {
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    inline std::string to_string(const boost::asio::ip::udp::endpoint& endpoint)
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
            return {{std::uint8_t(value)}};
        }
        else
        {
            std::array<std::uint8_t, sizeof(T)> retval;
            for (auto i = 0u; i < sizeof(T); ++i)
            {
                const auto index = (sizeof(T) - i - 1);
                retval[i] = std::uint8_t(value >> (index * TO_BITS));
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
                const auto index = (sizeof(T) - i - 1);
                t |= T(data[i]) << (index * TO_BITS);
            }
            return t;
        }
    }
}
