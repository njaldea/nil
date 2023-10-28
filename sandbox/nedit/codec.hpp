#pragma once

#include <nil/service.hpp>

#include <google/protobuf/message_lite.h>

namespace nil::service
{
    template <typename Message>
    struct codec<
        Message,
        std::enable_if_t<std::is_base_of_v<google::protobuf::MessageLite, Message>>>
    {
        static std::vector<std::uint8_t> serialize(const Message& message)
        {
            return codec<std::string>::serialize(message.SerializeAsString());
        }

        static Message deserialize(const void* data, std::uint64_t size)
        {
            Message m;
            m.ParseFromArray(data, int(size));
            return m;
        }
    };
}
