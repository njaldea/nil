#pragma once

#include <optional>
#include <string>

namespace nil::cli::conf
{
    struct Flag final
    {
        /**
         * @brief Short Key - Alias
         */
        std::optional<char> skey = {};

        /**
         * @brief Message to be used during help
         */
        std::optional<std::string> msg = {};
    };

    struct Number final
    {
        /**
         * @brief Short Key - Alias
         */
        std::optional<char> skey = {};

        /**
         * @brief Message to be used during help
         */
        std::optional<std::string> msg = {};

        /**
         * @brief Value used when option is not provided
         */
        std::int64_t fallback = 0;

        /**
         * @brief Value used when there is no value provided to the option
         */
        std::int64_t implicit = 1;
    };

    struct Param final
    {
        /**
         * @brief Short Key - Alias
         */
        std::optional<char> skey = {};

        /**
         * @brief Message to be used during help
         */
        std::optional<std::string> msg = {};

        /**
         * @brief Value used when option is not provided
         */
        std::optional<std::string> fallback = {};
    };

    struct Params final
    {
        /**
         * @brief Short Key - Alias
         */
        std::optional<char> skey = {};

        /**
         * @brief Message to be used during help
         */
        std::optional<std::string> msg = {};
    };
}
