#pragma once

#include <nil/service/codec.hpp>

#include <string>
#include <vector>

namespace nil::service
{
    template <typename T, typename = void>
    struct codec
    {
        static std::vector<std::uint8_t> serialize(const T&);
        static T deserialize(const void* data, std::uint64_t& size);
    };

    template <>
    std::vector<std::uint8_t> codec<std::string>::serialize(const std::string& message);
    template <>
    std::string codec<std::string>::deserialize(const void* data, std::uint64_t& size);

    template <>
    std::vector<std::uint8_t> codec<std::uint32_t>::serialize(const std::uint32_t& message);
    template <>
    std::uint32_t codec<std::uint32_t>::deserialize(const void* data, std::uint64_t& size);

    template <>
    std::vector<std::uint8_t> codec<std::int32_t>::serialize(const std::int32_t& message);
    template <>
    std::int32_t codec<std::int32_t>::deserialize(const void* data, std::uint64_t& size);
}
