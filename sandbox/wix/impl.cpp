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
    msg.set_min(data.constants.min);
    msg.set_max(data.constants.max);
    msg.set_label(data.constants.label);
    msg.set_value(data.value());
}

void apply(nil::wix::proto::Text& msg, const Text& data)
{
    msg.set_id(data.id);
    msg.set_value(data.value());
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
