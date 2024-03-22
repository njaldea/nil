#include <nil/service/codec.hpp>

#include "utils.hpp"

namespace nil::service
{
    template <>
    std::vector<std::uint8_t> codec<std::string>::serialize(const std::string& message)
    {
        return {message.begin(), message.end()};
    }

    template <>
    std::string codec<std::string>::deserialize(const void* data, std::uint64_t& size)
    {
        const auto o_size = size;
        size = 0;
        return {static_cast<const char*>(data), o_size};
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NIL_SERVICE_CODEC_DEFINE(TYPE)                                                             \
    template <>                                                                                    \
    std::vector<std::uint8_t> codec<TYPE>::serialize(const TYPE& message)                          \
    {                                                                                              \
        const auto typed = utils::to_array(message);                                               \
        return std::vector<std::uint8_t>(typed.begin(), typed.end());                              \
    }                                                                                              \
                                                                                                   \
    template <>                                                                                    \
    TYPE codec<TYPE>::deserialize(const void* data, std::uint64_t& size)                           \
    {                                                                                              \
        size -= sizeof(TYPE);                                                                      \
        return utils::from_array<TYPE>(static_cast<const std::uint8_t*>(data));                    \
    }

    NIL_SERVICE_CODEC_DEFINE(std::uint8_t);
    NIL_SERVICE_CODEC_DEFINE(std::uint16_t);
    NIL_SERVICE_CODEC_DEFINE(std::uint32_t);
    NIL_SERVICE_CODEC_DEFINE(std::uint64_t);

    NIL_SERVICE_CODEC_DEFINE(std::int8_t);
    NIL_SERVICE_CODEC_DEFINE(std::int16_t);
    NIL_SERVICE_CODEC_DEFINE(std::int32_t);
    NIL_SERVICE_CODEC_DEFINE(std::int64_t);
#undef NIL_SERVICE_CODEC_DEFINE
}
