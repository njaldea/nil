#include "dev.hpp"

#include <iostream>

namespace nil
{
    void log(const std::optional<std::string>& message, const std::source_location& location)
    {
        std::cout << location.file_name() << ':';
        std::cout << location.line() << ':';
        std::cout << location.column() << ':';
        std::cout << location.function_name();
        if (message)
        {
            std::cout << std::endl;
            std::cout << message.value();
        }
        std::cout << std::endl;
    }
}
