#pragma once

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

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NIL_SERVICE_CODEC_DECLARE(TYPE)                                                            \
    template <>                                                                                    \
    std::vector<std::uint8_t> codec<TYPE>::serialize(const TYPE& message);                         \
    template <>                                                                                    \
    TYPE codec<TYPE>::deserialize(const void* data, std::uint64_t& size)

    NIL_SERVICE_CODEC_DECLARE(std::string);

    NIL_SERVICE_CODEC_DECLARE(std::uint8_t);
    NIL_SERVICE_CODEC_DECLARE(std::uint16_t);
    NIL_SERVICE_CODEC_DECLARE(std::uint32_t);
    NIL_SERVICE_CODEC_DECLARE(std::uint64_t);

    NIL_SERVICE_CODEC_DECLARE(std::int8_t);
    NIL_SERVICE_CODEC_DECLARE(std::int16_t);
    NIL_SERVICE_CODEC_DECLARE(std::int32_t);
    NIL_SERVICE_CODEC_DECLARE(std::int64_t);
#undef NIL_SERVICE_CODEC_DECLARE
}
