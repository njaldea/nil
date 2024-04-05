#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

struct NodeInfo
{
    std::uint64_t id;                    // redundant to key. for convenience
    std::uint64_t type;                  // node type (index registered)
    std::vector<std::uint64_t> inputs;   // pin ids
    std::vector<std::uint64_t> outputs;  // pin ids
    std::vector<std::uint64_t> controls; // pin ids
};

struct LinkInfo
{
    std::uint64_t id;     // redundant to key. for convenience
    std::uint64_t type;   // pin type (index registered)
    std::uint64_t input;  // pin id
    std::uint64_t output; // pin id
};

struct GraphInfo
{
    template <typename T>
    using map_t = std::unordered_map<std::uint64_t, T>;
    template <typename T>
    using const_iterator = map_t<T>::const_iterator;

    map_t<NodeInfo> nodes;
    map_t<LinkInfo> links;
    map_t<const_iterator<NodeInfo>> pin_to_node;
    map_t<const_iterator<LinkInfo>> input_pin_to_link;
    std::multimap<std::uint64_t, map_t<NodeInfo>::const_iterator> scores;

    void add_node(NodeInfo info);
    void add_link(LinkInfo info);
    void score();

    std::uint64_t opposite_output_pin(std::uint64_t input_pin) const;
    std::uint64_t link_type(std::uint64_t input_pin) const;
};
