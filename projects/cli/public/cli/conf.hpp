#pragma once

#include <optional>
#include <string>

namespace nil::cli::conf
{
    struct Flag
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

    struct Number
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
        int fallback = 0;

        /**
         * @brief Value used when there is no value provided to the option
         */
        int implicit = 1;
    };

    struct Param
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

    struct Params
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
