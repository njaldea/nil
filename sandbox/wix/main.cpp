#include <nil/service.hpp>
#include <nil/service/ws/Server.hpp>

#include <wix/messages/sample.pb.h>

#include "serve.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>

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

struct Setters
{
    std::unordered_map<std::int32_t, std::function<void(std::int64_t)>*> ints;
    std::unordered_map<std::int32_t, std::function<void(std::string)>*> strings;
};

struct RangeConstants
{
    std::int64_t min;
    std::int64_t max;
    std::int32_t step;
    std::string label;
};

struct Range
{
    std::int32_t id;
    std::function<std::int64_t()> value;
    std::function<void(std::int64_t)> set_value;
    RangeConstants constants;
};

struct TextConstants
{
    std::string placeholder;
};

struct Text
{
    std::int32_t id;
    std::function<std::string()> value;
    std::function<void(std::string)> set_value;
    TextConstants constants;
};

struct Block
{
    std::string label;
    std::vector<std::variant<Block, Range, Text>> contents;
};

void apply(nil::wix::proto::Block& msg, const Range& data)
{
    auto* r = msg.add_widgets()->mutable_range();
    r->set_id(data.id);
    r->set_min(data.constants.min);
    r->set_max(data.constants.max);
    r->set_label(data.constants.label);
    r->set_value(data.value());
}

void apply(nil::wix::proto::Block& msg, const Text& data)
{
    auto* t = msg.add_widgets()->mutable_text();
    t->set_id(data.id);
    t->set_value(data.value());
    t->set_placeholder(data.constants.placeholder);
}

void apply(nil::wix::proto::Block& msg, const Block& data)
{
    auto* b = msg.add_widgets()->mutable_block();
    b->set_label(data.label);
    for (const auto& content : data.contents)
    {
        std::visit([&](const auto& c) { ::apply(*b, c); }, content);
    }
}

void apply(nil::wix::proto::Wix& msg, const Block& data)
{
    auto* child = msg.add_blocks();
    child->set_label(data.label);
    for (const auto& content : data.contents)
    {
        std::visit([&](const auto& c) { ::apply(*child, c); }, content);
    }
}

Block& add_block(Block& block, std::string label)
{
    return std::get<Block>(block.contents.emplace_back(Block(std::move(label), {})));
}

void add_range(
    Block& block,
    std::int32_t id,
    std::function<std::int64_t()> value,
    std::function<void(std::int64_t)> set_value,
    RangeConstants constants
)
{
    block.contents.emplace_back(
        Range(id, std::move(value), std::move(set_value), std::move(constants))
    );
}

void add_text(
    Block& block,
    std::int32_t id,
    std::function<std::string()> value,
    std::function<void(std::string)> set_value,
    TextConstants constants
)
{
    block.contents.emplace_back(
        Text(id, std::move(value), std::move(set_value), std::move(constants))
    );
}

auto install(Block& block, std::int32_t& next_id)
{
    add_range(block, next_id++, []() { return 5; }, [](std::int64_t) {}, {1, 10, 1, "first range"});
    add_range(
        block,
        next_id++,
        []() { return 5; },
        [](std::int64_t) {},
        {2, 20, 2, "second range"}
    );
    add_text(block, next_id++, []() { return "text here"; }, [](const auto&) {}, {"what to do"});
    add_text(block, next_id++, []() { return ""; }, [](const auto&) {}, {"empty value"});
}

int main()
{
    std::int32_t next_id = 0;
    Block block1 = {"top block", {}};
    install(block1, next_id);
    install(add_block(block1, "bottom block"), next_id);

    std::thread http(serve);

    nil::service::ws::Server service({.port = 1101});
    service.on_connect(                                 //
        [&service, &block1](const nil::service::ID& id) //
        {
            std::cout << "connect: " << id.text << std::endl;
            nil::wix::proto::Wix msg;
            apply(msg, block1);

            service.send(id, nil::service::concat(nil::wix::proto::MessageType_Wix, msg));
        }
    );
    service.on_disconnect(             //
        [](const nil::service::ID& id) //
        {                              //
            std::cout << "disconnect: " << id.text << std::endl;
        }
    );
    service.on_message(
        [](const nil::service::ID& id, const void* data, std::uint64_t size)
        {
            (void)id;
            switch (nil::service::type_cast<nil::wix::proto::MessageType>(data, size))
            {
                case nil::wix::proto::MessageType_I64Update:
                    std::cout << "i64 update" << std::endl;
                    break;
                case nil::wix::proto::MessageType_TextUpdate:
                    std::cout << "txt update" << std::endl;
                    break;
                default:
                    break;
            }
        }
    );
    service.run();
    return 0;
}
