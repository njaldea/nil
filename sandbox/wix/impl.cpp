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
        std::visit([&](const auto& c) { ::apply(::create(msg, c), c); }, *content);
    }
}

Range& add_range(
    Mutator& mutator,
    Block& block,
    std::int32_t id,
    std::int64_t value,
    Range::Constants constants
)
{
    auto& object = std::get<Range>(   //
        *block.contents.emplace_back( //
            std::make_unique<Block::content_t>(Range(id, value, std::move(constants)))
        )
    );
    mutator.mutators.emplace( //
        id,
        [&object](auto v) //
        { object.value = std::get<std::int64_t>(v); }
    );
    return object;
}

Text& add_text(
    Mutator& mutator,
    Block& block,
    std::int32_t id,
    std::string value,
    Text::Constants constants
)
{
    auto& object = std::get<Text>(    //
        *block.contents.emplace_back( //
            std::make_unique<Block::content_t>(Text(id, std::move(value), std::move(constants)))
        )
    );
    mutator.mutators.emplace( //
        id,                   //
        [&object](auto v)     //
        { object.value = std::get<std::string>(std::move(v)); }
    );
    return object;
}

Block& add_block(Mutator& mutator, Block& block, std::string label)
{
    (void)mutator;
    auto& object = std::get<Block>(   //
        *block.contents.emplace_back( //
            std::make_unique<Block::content_t>(Block(std::move(label), {}))
        )
    );
    return object;
}
