#pragma once

#include <nil/gatex/Info.hpp>

namespace nil::nedit::proto
{
    class State;
}

namespace nil::gatex
{
    nil::gatex::GraphInfo parse(const nil::nedit::proto::State& state);
}
