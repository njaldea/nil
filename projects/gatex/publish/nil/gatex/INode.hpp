#pragma once

#include "Info.hpp"

namespace nil::gatex
{
    class Core;

    struct INode
    {
        INode() = default;
        virtual ~INode() = default;
        INode(INode&&) = delete;
        INode(const INode&) = delete;
        INode& operator=(INode&&) = delete;
        INode& operator=(const INode&) = delete;

        virtual void create_node(
            Core& xcore,
            const GraphInfo& graph,
            const NodeInfo& node_info //
        ) = 0;
    };
}
