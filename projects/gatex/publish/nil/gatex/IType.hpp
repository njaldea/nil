#pragma once

#include "Info.hpp"

namespace nil::gatex
{
    class Core;

    struct IType
    {
        IType() = default;
        virtual ~IType() = default;
        IType(IType&&) = delete;
        IType(const IType&) = delete;
        IType& operator=(IType&&) = delete;
        IType& operator=(const IType&) = delete;

        virtual void create_edge(
            Core& xcore,
            std::uint64_t id //
        ) = 0;
        virtual void create_feedback(
            Core& xcore,
            const GraphInfo& info,
            const NodeInfo& node_info //
        ) = 0;
        virtual void create_delay(
            Core& xcore,
            const GraphInfo& info,
            const NodeInfo& node_info //
        ) = 0;
    };
}
