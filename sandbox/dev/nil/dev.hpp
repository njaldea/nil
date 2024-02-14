#include <optional>
#include <source_location>
#include <string>

namespace nil
{
    void log(
        const char* message = nullptr,
        const std::source_location& location = std::source_location::current()
    );
}
