#include <nil/gatex/Core.hpp>
#include <nil/gatex/parse.hpp>
#include <nil/gatex/serve.hpp>

#include <nil/service.hpp>

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
        auto server = nil::service::tcp::server::create({.port = port});

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
                publish(
                    server,
                    nil::service::concat(nil::nedit::proto::message_type::NodeState, message)
                );
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        );

        core.set_after_hook(
            [&server](std::uint64_t id)
            {
                nil::nedit::proto::NodeState message;
                message.set_id(id);
                message.set_active(false);
                publish(
                    server,
                    nil::service::concat(nil::nedit::proto::message_type::NodeState, message)
                );
            }
        );

        on_connect(
            server,
            [&server, &core, &state](const nil::service::ID& id)
            {
                core.stop();
                send(
                    server,
                    id,
                    nil::service::concat(nil::nedit::proto::message_type::State, state)
                );
            }
        );

        namespace proto = nil::nedit::proto;
        on_message(
            server,
            nil::service::map(
                nil::service::mapping(
                    proto::message_type::ControlUpdateB,
                    [&](const proto::ControlUpdateB& msg)
                    { core.set_control_value(msg.id(), msg.value()); }
                ),
                nil::service::mapping(
                    proto::message_type::ControlUpdateI,
                    [&](const proto::ControlUpdateI& msg)
                    { core.set_control_value(msg.id(), msg.value()); }
                ),
                nil::service::mapping(
                    proto::message_type::ControlUpdateF,
                    [&](const proto::ControlUpdateF& msg)
                    { core.set_control_value(msg.id(), msg.value()); }
                ),
                nil::service::mapping(
                    proto::message_type::ControlUpdateS,
                    [&](const proto::ControlUpdateS& msg)
                    { core.set_control_value(msg.id(), msg.value()); }
                ),
                nil::service::mapping(proto::message_type::Play, [&]() { core.resume(); }),
                nil::service::mapping(proto::message_type::Pause, [&]() { core.stop(); }),
                nil::service::mapping(
                    proto::message_type::State,
                    [&](const auto& id, const proto::State& msg)
                    {
                        if (!core.is_compatible(msg.types().SerializeAsString()))
                        {
                            std::cerr << "state is not compatible to types" << std::endl;
                            return;
                        }
                        core.stop();
                        core.wait();
                        core.instantiate(parse(msg));
                        send(server, id, nil::service::concat(proto::message_type::State, msg));
                    }
                ),
                nil::service::mapping(
                    proto::message_type::Run,
                    [&]()
                    {
                        core.stop();
                        core.wait();
                        core.start();
                    }
                )
            )
        );

        start(server);
    }
}
