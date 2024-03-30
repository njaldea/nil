#include "dev.hpp"

#include <format>
#include <iostream>

namespace nil
{
    void log(const char* message, const std::source_location& location)
    {
        if (nullptr == message)
        {
            std::cout //
                << std::format(
                       "{}:{}:{}:{}\n",
                       location.file_name(),
                       location.line(),
                       location.column(),
                       location.function_name()
                   )
                << std::flush;
        }
        else
        {
            std::cout //
                << std::format(
                       "{}:{}:{}:{}\n{}\n",
                       location.file_name(),
                       location.line(),
                       location.column(),
                       location.function_name(),
                       message
                   )
                << std::flush;
        }
    }
}
