#pragma once

#include <cstdint>
#include <vector>

namespace ext
{
    /**
     * @brief This is a node representation without regard to the type of edges.
     *      `std::uint64_t` represents the ID of the edges each input/output are connected to.
     */
    struct NodeData final
    {
        // node's index when registered to builder
        std::uint64_t type;
        std::vector<std::uint64_t> inputs;
        std::vector<std::uint64_t> outputs;
        std::vector<std::uint64_t> controls;
    };

    std::vector<NodeData> sort_by_score(const std::vector<NodeData>& nodes);
}
