#include <nil/service.hpp>

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

nil::wix::proto::Range& create(nil::wix::proto::Block& block, const Range&)
{
    return *block.add_widgets()->mutable_range();
}

void apply(nil::wix::proto::Range& msg, const Range& data)
{
    msg.set_id(data.id);
    msg.set_min(data.constants.min);
    msg.set_max(data.constants.max);
    msg.set_label(data.constants.label);
    msg.set_value(data.value());
}

nil::wix::proto::Text& create(nil::wix::proto::Block& block, const Text&)
{
    return *block.add_widgets()->mutable_text();
}

void apply(nil::wix::proto::Text& msg, const Text& data)
{
    msg.set_id(data.id);
    msg.set_value(data.value());
    msg.set_placeholder(data.constants.placeholder);
}

nil::wix::proto::Block& create(nil::wix::proto::Block& block, const Block&)
{
    return *block.add_widgets()->mutable_block();
}

void apply(nil::wix::proto::Block& msg, const Block& data)
{
    msg.set_label(data.label);
    for (const auto& content : data.contents)
    {
        std::visit([&](const auto& c) { ::apply(::create(msg, c), c); }, content);
    }
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

Block& add_block(Block& block, std::string label)
{
    return std::get<Block>(block.contents.emplace_back(Block(std::move(label), {})));
}

void install(Block& block, std::int32_t& next_id)
{
    add_range(block, next_id++, []() { return 5; }, [](std::int64_t) {}, {1, 10, 1, "range[1]"});
    add_range(block, next_id++, []() { return 5; }, [](std::int64_t) {}, {2, 20, 2, "range[2]"});
    add_text(block, next_id++, []() { return "text here"; }, [](const auto&) {}, {"what to do"});
    add_text(block, next_id++, []() { return ""; }, [](const auto&) {}, {"empty value"});
}

int main()
{
    std::cout << "http://localhost:8080" << std::endl;

    std::int32_t next_id = 0;
    Block block1 = {"top block", {}};
    install(block1, next_id);
    install(add_block(block1, "bottom block"), next_id);

    std::thread http(serve);

    nil::service::ws::Server service({.port = 1101});
    service.on_connect(                     //
        [&service, &block1](const auto& id) //
        {
            std::cout << "connect: " << id.text << std::endl;
            nil::wix::proto::Wix msg;
            apply(*msg.add_blocks(), block1);

            service.send(id, nil::service::concat(nil::wix::proto::MessageType_Wix, msg));
        }
    );
    service.on_disconnect( //
        [](const auto& id) //
        {                  //
            std::cout << "disconnect: " << id.text << std::endl;
        }
    );
    service.on_message(
        [](const auto& id, const void* data, std::uint64_t size)
        {
            (void)id;
            switch (nil::service::consume<nil::wix::proto::MessageType>(data, size))
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
