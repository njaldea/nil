#pragma once

#include <cstdint>

namespace nil::gatex
{
    class Core;
    void serve(Core& core, std::uint16_t port);
}
