#include "dev.hpp"

#include <cstdio>

namespace nil
{
    void log(const char* message, const std::source_location& location)
    {
        if (nullptr == message)
        {
            std::printf(
                "%s:%d:%d:%s\n",
                location.file_name(),
                location.line(),
                location.column(),
                location.function_name()
            );
        }
        else
        {
            std::printf(
                "%s:%d:%d:%s\n%s\n",
                location.file_name(),
                location.line(),
                location.column(),
                location.function_name(),
                message
            );
        }
    }
}
