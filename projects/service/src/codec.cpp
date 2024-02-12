#include <nil/service/codec.hpp>

#include "utils.hpp"

namespace nil::service
{
    template <>
    std::vector<std::uint8_t> codec<std::string>::serialize(const std::string& message)
    {
        return std::vector<std::uint8_t>(message.begin(), message.end());
    }

    template <>
    std::vector<std::uint8_t> codec<std::uint32_t>::serialize(const std::uint32_t& message)
    {
        const auto typed = utils::to_array(message);
        return std::vector<std::uint8_t>(typed.begin(), typed.end());
    }

    template <>
    std::vector<std::uint8_t> codec<std::int32_t>::serialize(const std::int32_t& message)
    {
        const auto typed = utils::to_array(message);
        return std::vector<std::uint8_t>(typed.begin(), typed.end());
    }

    template <>
    std::string codec<std::string>::deserialize(const void* data, std::uint64_t& size)
    {
        size = 0;
        return std::string(static_cast<const char*>(data), size);
    }

    template <>
    std::uint32_t codec<std::uint32_t>::deserialize(const void* data, std::uint64_t& size)
    {
        // make sure to pass the right size
        size -= sizeof(std::uint32_t);
        return utils::from_array<std::uint32_t>(static_cast<const std::uint8_t*>(data));
    }

    template <>
    std::int32_t codec<std::int32_t>::deserialize(const void* data, std::uint64_t& size)
    {
        // make sure to pass the right size
        size -= sizeof(std::uint32_t);
        return utils::from_array<std::int32_t>(static_cast<const std::uint8_t*>(data));
    }
}
