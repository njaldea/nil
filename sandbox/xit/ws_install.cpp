#include "ws_install.hpp"

#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <xit/messages/message.pb.h>

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
    struct codec<nil::xit::proto::MessageType>
    {
        using type = nil::xit::proto::MessageType;

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
    server.on_ready(                                                               //
        [](const auto& id)                                                         //
        {                                                                          //
            std::cout << "ready ws      : " << id.text << std::endl;               //
            std::cout << " ui is at     : https://xit-ui.vercel.app" << std::endl; //
        }
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
            namespace nwp = nil::xit::proto;
            const auto message_type = nil::service::consume<nwp::MessageType>(data, size);
            std::cout << "received: " << id.text << ":" << message_type << std::endl;
            switch (message_type)
            {
                case nwp::MessageType_MarkupRequest:
                {
                    nwp::MarkupResponse response;
                    *response.add_components()
                        = "/home/njaldea/repo/cpp/nil/sandbox/xit/gui/Markup.svelte";
                    *response.add_components()
                        = "/home/njaldea/repo/cpp/nil/sandbox/xit/gui/Markup.svelte";
                    *response.add_components()
                        = "/home/njaldea/repo/cpp/nil/sandbox/xit/gui/Markup.svelte";
                    server.send(
                        id,
                        nil::service::concat(nwp::MessageType_MarkupResponse, response)
                    );
                    break;
                }
                case nwp::MessageType_BindingRequest:
                {
                    nwp::BindingResponse response;
                    auto* info = response.add_info();
                    {
                        auto* binding = info->add_bindings();
                        binding->set_tag("binding_0_0");
                        binding->set_value_i64(1000);
                    }
                    {
                        auto* binding = info->add_bindings();
                        binding->set_tag("binding_0_1");
                        binding->set_value_str("hello world");
                    }
                    response.add_info();
                    response.add_info();
                    server.send(
                        id,
                        nil::service::concat(nwp::MessageType_BindingResponse, response)
                    );
                    break;
                }
                case nwp::MessageType_BindingUpdate:
                {
                    const auto msg = nil::service::consume<nwp::Binding>(data, size);
                    std::cout << "binding update: " << std::endl;
                    std::cout << " -  " << msg.tag() << std::endl;
                    if (msg.has_value_i64())
                    {
                        std::cout << "i: -  " << msg.value_i64() << std::endl;
                    }
                    if (msg.has_value_str())
                    {
                        std::cout << "s: -  " << msg.value_str() << std::endl;
                    }
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
                    break;
                }
                default:
                    break;
            }
        }
    );
}
