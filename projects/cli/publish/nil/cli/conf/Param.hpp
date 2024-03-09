#pragma once

#include <optional>
#include <string>

namespace nil::cli::conf
{
    struct Param final
    {
        /**
         * @brief Short Key - Alias
         */
        std::optional<char> skey;

        /**
         * @brief Message to be used during help
         */
        std::optional<std::string> msg;

        /**
         * @brief Value used when option is not provided
         */
        std::optional<std::string> fallback;
    };
}
