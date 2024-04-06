#include "Command.hpp"

#include <nil/dev.hpp>
#include <nil/gatex.hpp>
#include <nil/service.hpp>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

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

nil::cli::OptionInfo CMD::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "use port", .fallback = 1101})
        .build();
}

template <typename T>
struct Input
{
    T operator()(const T& value) const
    {
        std::cout << std::format("Input[{}]: <{}>\n", name, value) << std::flush;
        return value;
    }

    std::string name;
};

struct Add
{
    int operator()(int l, int r) const
    {
        std::cout << std::format("Add: {} + {} = {}\n", l, r, l + r) << std::flush;
        return l + r;
    }
};

struct Mul
{
    int operator()(int l, int r) const
    {
        std::cout << std::format("Mul: {} * {} = {}\n", l, r, l * r) << std::flush;
        return l * r;
    }
};

struct Inverter
{
    int operator()(bool l, int r) const
    {
        std::cout << std::format("Inv: {} => {}\n", l ? 'T' : 'F', l ? -r : r) << std::flush;
        return l ? -r : r;
    }
};

template <typename T>
struct Consume
{
    void operator()(const T& v) const
    {
        std::cout << std::format("Consume: {}\n", v) << std::flush;
    }
};

int CMD::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    std::string file = "../sandbox/nedit/state.dump";
    if (!std::filesystem::exists(file))
    {
        return 1;
    }

    nil::gatex::Core core;
    {
        core.add_type(false);
        core.add_type(0);
        core.add_type(0.0f);
        core.add_type(std::string());

        core.add_node(Input<bool>("b"), false);
        core.add_node(Input<int>("i"), 5);
        core.add_node(Input<float>("f"), 0.5f);
        core.add_node(Input<std::string>("i"), std::string());

        core.add_node(Inverter());

        core.add_node(Add());
        core.add_node(Add(), 5);
        core.add_node(Mul());
        core.add_node(Mul(), 5);

        core.add_node(Consume<bool>());
        core.add_node(Consume<int>());
        core.add_node(Consume<float>());
        core.add_node(Consume<std::string>());
    }

    core.instantiate(nil::gatex::load(file));

    nil::gatex::serve(core, 1101);

    return 0;
}
