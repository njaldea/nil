#pragma once

#include <nil/service/codec.hpp>

#include <wix/messages/sample.pb.h>

#include <variant>

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

nil::wix::proto::Range& create(nil::wix::proto::Block& block, const Range&);
nil::wix::proto::Text& create(nil::wix::proto::Block& block, const Text&);
nil::wix::proto::Block& create(nil::wix::proto::Block& block, const Block&);

void apply(nil::wix::proto::Range& msg, const Range& data);
void apply(nil::wix::proto::Text& msg, const Text& data);
void apply(nil::wix::proto::Block& msg, const Block& data);

void add_range(
    Block& block,
    std::int32_t id,
    std::function<std::int64_t()> value,
    std::function<void(std::int64_t)> set_value,
    RangeConstants constants
);
void add_text(
    Block& block,
    std::int32_t id,
    std::function<std::string()> value,
    std::function<void(std::string)> set_value,
    TextConstants constants
);
Block& add_block(Block& block, std::string label);

void install(Block& block, std::int32_t& next_id);
