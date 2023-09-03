#pragma once

#include "Builder.hpp"
#include "Options.hpp"

#include <string>

namespace nil::cli
{
    /**
     * @brief Class that user's have to inherit from to enforce
     *  API and provide default behavior.
     */
    struct Command
    {
        virtual ~Command() = default;

        /**
         * @brief description to be use when printing Command
         *  for as a sub command.
         *
         * @return std::string
         */
        virtual std::string description() const
        {
            return "";
        }

        /**
         * @brief message to use used when printing help for this command.
         *
         * @return std::string
         */
        virtual std::string usage() const
        {
            return "";
        }

        /**
         * @brief option descriptions. Use Builder to populate OptionInfo.
         *
         * @return OptionInfo
         */
        virtual OptionInfo options() const
        {
            return {};
        }

        /**
         * @brief Method to be executed after parsing of arg options.
         *
         * @param options
         * @return int
         */
        virtual int run(const Options& /* options */) const
        {
            return 0;
        }
    };
}
