#include "dev.hpp"

#include <iostream>

namespace nil
{
    void log(const char* message, const std::source_location& location)
    {
        std::cout                          //
            << location.file_name() << ':' //
            << location.line() << ':'      //
            << location.column() << ':'    //
            << location.function_name();
        if (message)
        {
            std::cout << '\n' << message;
        }
        std::cout << std::endl;
    }
}
