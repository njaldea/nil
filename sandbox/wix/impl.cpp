#include "impl.hpp"

nil::wix::proto::Range& create(nil::wix::proto::Block& block, const Range& /* tag */)
{
    return *block.add_widgets()->mutable_range();
}

nil::wix::proto::Text& create(nil::wix::proto::Block& block, const Text& /* tag */)
{
    return *block.add_widgets()->mutable_text();
}

nil::wix::proto::Block& create(nil::wix::proto::Block& block, const Block& /* tag */)
{
    return *block.add_widgets()->mutable_block();
}

void apply(nil::wix::proto::Range& msg, const Range& data)
{
    msg.set_id(data.id);
    msg.set_label(data.constants.label);
    msg.set_value(data.value);
    msg.set_min(data.constants.min);
    msg.set_max(data.constants.max);
}

void apply(nil::wix::proto::Text& msg, const Text& data)
{
    msg.set_id(data.id);
    msg.set_label(data.constants.label);
    msg.set_value(data.value);
    msg.set_placeholder(data.constants.placeholder);
}

void apply(nil::wix::proto::Block& msg, const Block& data)
{
    msg.set_label(data.label);
    for (const auto& content : data.contents)
    {
        std::visit([&](const auto& c) { ::apply(::create(msg, c), c); }, content);
    }
}

void add_range(Block& block, std::int32_t id, std::int64_t value, RangeConstants constants)
{
    block.contents.emplace_back(Range(id, value, std::move(constants)));
}

void add_text(Block& block, std::int32_t id, std::string value, TextConstants constants)
{
    block.contents.emplace_back(Text(id, std::move(value), std::move(constants)));
}

Block& add_block(Block& block, std::string label)
{
    return std::get<Block>(block.contents.emplace_back(Block(std::move(label), {})));
}

void install(Block& block, std::int32_t& next_id)
{
    add_range(block, next_id++, 5, {"range[1]", 1, 10, 1});
    add_range(block, next_id++, 5, {"range[2]", 2, 20, 2});
    add_text(block, next_id++, "text here", {"placeholder 1", "what to do"});
    add_text(block, next_id++, "", {"placeholder 2", "empty value"});
}
