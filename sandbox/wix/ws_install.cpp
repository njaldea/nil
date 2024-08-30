#include "ws_install.hpp"

#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <wix/messages/message.pb.h>

namespace nil::service
{
    template <typename Message>
    constexpr auto is_message_lite = std::is_base_of_v<google::protobuf::MessageLite, Message>;

    template <typename Message>
    struct codec<Message, std::enable_if_t<is_message_lite<Message>>>
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
    struct codec<nil::wix::proto::MessageType>
    {
        using type = nil::wix::proto::MessageType;

        static std::vector<std::uint8_t> serialize(const type& message)
        {
            return codec<std::uint32_t>::serialize(std::uint32_t(message));
        }

        static type deserialize(const void* data, std::uint64_t& size)
        {
            return type(codec<std::uint32_t>::deserialize(data, size));
        }
    };
}

void ws_install(nil::service::IService& server, Block& block)
{
    server.on_ready(                                                 //
        [](const auto& id)                                           //
        { std::cout << "ready ws      : " << id.text << std::endl; } //
    );
    server.on_connect(                    //
        [&server, &block](const auto& id) //
        {
            std::cout << "connect ws    : " << id.text << std::endl;
            nil::wix::proto::Wix msg;
            apply(*msg.add_blocks(), block);

            server.send(id, nil::service::concat(nil::wix::proto::MessageType_Wix, msg));
        }
    );
    server.on_disconnect(                                            //
        [](const auto& id)                                           //
        { std::cout << "disconnect ws : " << id.text << std::endl; } //
    );
    server.on_message(
        [](const void* data, std::uint64_t size)
        {
            namespace nwp = nil::wix::proto;
            switch (nil::service::consume<nwp::MessageType>(data, size))
            {
                case nwp::MessageType_I64Update:
                {
                    const auto msg = nil::service::consume<nwp::I64Update>(data, size);
                    std::cout << "i64 update: " << msg.id() << ":" << msg.value() << std::endl;
                    break;
                }
                case nwp::MessageType_StringUpdate:
                {
                    const auto msg = nil::service::consume<nwp::StringUpdate>(data, size);
                    std::cout << "str update: " << msg.id() << ":" << msg.value() << std::endl;
                    break;
                }
                default:
                    break;
            }
        }
    );
}
