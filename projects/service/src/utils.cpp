#include "utils.hpp"

#include <nil/service/TypedService.hpp>

namespace nil::service
{
    std::uint32_t TypedService::type(const void* data)
    {
        return utils::from_array<std::uint32_t>(static_cast<const std::uint8_t*>(data));
    }

    std::vector<std::uint8_t> TypedService::serialize(
        std::uint32_t type,
        std::vector<std::uint8_t> payload
    )
    {
        const auto typed = utils::to_array(type);
        std::vector<std::uint8_t> m;
        m.resize(payload.size() + typed.size());
        std::memcpy(m.data(), typed.data(), typed.size());
        std::memcpy(m.data() + typed.size(), payload.data(), payload.size());
        return m;
    }
}
