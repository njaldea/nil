#include "ws_install.hpp"

#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <wix/messages/message.pb.h>

#include <fstream>

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

void ws_install(nil::service::IService& server)
{
    server.on_ready(                                                 //
        [](const auto& id)                                           //
        { std::cout << "ready ws      : " << id.text << std::endl; } //
    );
    server.on_connect(                                               //
        [](const auto& id)                                           //
        { std::cout << "connect ws    : " << id.text << std::endl; } //
    );
    server.on_disconnect(                                            //
        [](const auto& id)                                           //
        { std::cout << "disconnect ws : " << id.text << std::endl; } //
    );
    server.on_message(
        [&server](const auto& id, const void* data, std::uint64_t size)
        {
            namespace nwp = nil::wix::proto;
            const auto message_type = nil::service::consume<nwp::MessageType>(data, size);
            std::cout << "received: " << id.text << ":" << message_type << std::endl;
            switch (message_type)
            {
                case nwp::MessageType_MarkupRequest:
                {
                    nil::wix::proto::MarkupResponse response;
                    *response.add_components()
                        = "/home/njaldea/repo/cpp/nil/sandbox/wix/gui/Markup.svelte";
                    server.send(
                        id,
                        nil::service::concat(nwp::MessageType_MarkupResponse, response)
                    );
                    break;
                }
                case nwp::MessageType_FileRequest:
                {
                    const auto msg = nil::service::consume<nwp::FileRequest>(data, size);
                    nwp::FileResponse response;
                    response.set_target(msg.target());
                    std::cout << "file request: " << msg.target() << std::endl;
                    std::fstream file(msg.target());
                    *response.mutable_content() = std::string(
                        std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>()
                    );
                    server.send(id, nil::service::concat(nwp::MessageType_FileResponse, response));
                }
                default:
                    break;
            }
        }
    );
}
