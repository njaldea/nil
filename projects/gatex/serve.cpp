#include <nil/gatex/Core.hpp>
#include <nil/gatex/parse.hpp>
#include <nil/gatex/serve.hpp>

#include <nil/service/TypedHandler.hpp>
#include <nil/service/tcp/Server.hpp>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

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
    struct codec<nil::nedit::proto::message_type::MessageType>
    {
        using type = nil::nedit::proto::message_type::MessageType;

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

namespace nil::gatex
{
    void serve(Core& core, std::uint16_t port)
    {
        nil::service::tcp::Server server({.port = port});

        nil::nedit::proto::State state;
        // TODO: temporary. this should be owned by core for tracking.. but idk how it should work.
        std::ifstream fs("../sandbox/nedit/state.dump", std::ios::binary);
        state.ParseFromIstream(&fs);

        core.set_before_hook(
            [&server](std::uint64_t id)
            {
                nil::nedit::proto::NodeState message;
                message.set_id(id);
                message.set_active(true);
                server.publish(nil::nedit::proto::message_type::NodeState, message);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        );

        core.set_after_hook(
            [&server](std::uint64_t id)
            {
                nil::nedit::proto::NodeState message;
                message.set_id(id);
                message.set_active(false);
                server.publish(nil::nedit::proto::message_type::NodeState, message);
            }
        );

        server.on_connect( //
            [&server, &core, &state](const std::string& id)
            {
                core.stop();
                server.send(id, nil::nedit::proto::message_type::State, state);
            }
        );

        namespace proto = nil::nedit::proto;
        server.on_message(                                                 //
            nil::service::TypedHandler<proto::message_type::MessageType>() //
                .add(
                    proto::message_type::ControlUpdateB,
                    [&core](const std::string& /* id */, const proto::ControlUpdateB& message)
                    { core.set_control_value(message.id(), message.value()); }
                )
                .add(
                    proto::message_type::ControlUpdateI,
                    [&core](const std::string& /* id */, const proto::ControlUpdateI& message)
                    { core.set_control_value(message.id(), message.value()); }
                )
                .add(
                    proto::message_type::ControlUpdateF,
                    [&core](const std::string& /* id */, const proto::ControlUpdateF& message)
                    { core.set_control_value(message.id(), message.value()); }
                )
                .add(
                    proto::message_type::ControlUpdateS,
                    [&core](const std::string& /* id */, const proto::ControlUpdateS& message)
                    { core.set_control_value(message.id(), message.value()); }
                )
                .add(proto::message_type::Play, [&core]() { core.resume(); })
                .add(proto::message_type::Pause, [&core]() { core.stop(); })
                .add(
                    proto::message_type::State,
                    [&server, &core](const std::string& id, const proto::State& msg)
                    {
                        if (!core.is_compatible(msg.types().SerializeAsString()))
                        {
                            std::cerr << "state is not compatible to types" << std::endl;
                            return;
                        }
                        core.stop();
                        core.wait();
                        core.instantiate(parse(msg));
                        server.send(id, proto::message_type::State, msg);
                    }
                )
                .add(
                    proto::message_type::Run,
                    [&core]()
                    {
                        core.stop();
                        core.wait();
                        core.start();
                    }
                )
        );

        server.run();
    }
}
