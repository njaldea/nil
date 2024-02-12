#pragma once

#include <nil/service/codec.hpp>

#include <google/protobuf/message_lite.h>

#include <gen/nedit/messages/type.pb.h>

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

        static Message deserialize(const void* data, std::uint64_t& size)
        {
            Message m;
            m.ParseFromArray(data, int(size));
            size = 0;
            return m;
        }
    };

    template <>
    struct codec<nil::nedit::proto::message_type::MessageType>
    {
        static std::vector<std::uint8_t> serialize(
            const nil::nedit::proto::message_type::MessageType& message
        )
        {
            return codec<std::uint32_t>::serialize(message);
        }

        static nil::nedit::proto::message_type::MessageType deserialize(
            const void* data,
            std::uint64_t& size
        )
        {
            return nil::nedit::proto::message_type::MessageType(
                codec<std::uint32_t>::deserialize(data, size)
            );
        }
    };
}
