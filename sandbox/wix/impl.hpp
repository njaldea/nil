#pragma once

#include <nil/service/codec.hpp>

#include <wix/messages/message.pb.h>

#include <variant>

struct Range
{
    std::int32_t id;
    std::int64_t value;

    struct Constants
    {
        std::string label;
        std::int64_t min;
        std::int64_t max;
        std::int32_t step;
    } constants;
};

struct Text
{
    std::int32_t id;
    std::string value;

    struct Constants
    {
        std::string label;
        std::string placeholder;
    } constants;
};

struct Block
{
    std::string label;
    using content_t = std::variant<Block, Range, Text>;
    std::vector<std::unique_ptr<content_t>> contents;
};

struct Mutator
{
    using type = std::variant<std::int64_t, std::string>;
    std::unordered_map<std::int32_t, std::function<void(type)>> mutators;
};

nil::wix::proto::Range& create(nil::wix::proto::Block& block, const Range&);
nil::wix::proto::Text& create(nil::wix::proto::Block& block, const Text&);
nil::wix::proto::Block& create(nil::wix::proto::Block& block, const Block&);

void apply(nil::wix::proto::Range& msg, const Range& data);
void apply(nil::wix::proto::Text& msg, const Text& data);
void apply(nil::wix::proto::Block& msg, const Block& data);

Range& add_range(
    Mutator& mutator,
    Block& block,
    std::int32_t id,
    std::int64_t value,
    Range::Constants constants
);
Text& add_text(
    Mutator& mutator,
    Block& block,
    std::int32_t id,
    std::string value,
    Text::Constants constants
);
Block& add_block(Mutator& mutator, Block& block, std::string label);
