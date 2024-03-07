#pragma once

#include <source_location>

namespace nil
{
    void log(
        const char* message = nullptr,
        const std::source_location& location = std::source_location::current()
    );
}
