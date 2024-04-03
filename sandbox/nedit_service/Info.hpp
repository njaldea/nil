#pragma once

#include <gen/nedit/messages/state.pb.h>

#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

struct NodeInfo
{
    std::uint64_t type;
    std::vector<std::uint64_t> inputs;
    std::vector<std::uint64_t> outputs;
    std::vector<std::uint64_t> controls;
};

struct LinkInfo
{
    std::uint64_t input;
    std::uint64_t output;
    std::uint64_t type;
};

template <typename T>
struct PinInfo
{
    std::unordered_map<std::uint64_t, T>::const_iterator it;
};

struct GraphInfo
{
    template <typename T>
    using map_t = std::unordered_map<std::uint64_t, T>;

    map_t<NodeInfo> nodes;
    map_t<LinkInfo> links;
    map_t<PinInfo<NodeInfo>> pin_to_node;
    map_t<PinInfo<LinkInfo>> pin_to_link;
    std::multimap<std::uint64_t, map_t<NodeInfo>::const_iterator> scores;
};

GraphInfo populate(const nil::nedit::proto::State& state);
