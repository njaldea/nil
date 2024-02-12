#include <optional>
#include <source_location>
#include <string>

namespace nil
{
    void log(
        const std::optional<std::string>& message = std::nullopt,
        const std::source_location& location = std::source_location::current()
    );
}
