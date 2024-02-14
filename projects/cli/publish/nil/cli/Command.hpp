#pragma once

#include "IOption.hpp"
#include "types.hpp"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace nil::cli
{
    class Options;

    /**
     * @brief Class that user's have to inherit from to enforce
     *  API and provide default behavior.
     */
    struct Command
    {
        virtual ~Command() noexcept = default;

        /**
         * @brief message to use used when printing help for this command.
         *
         * @note [TODO] receive a text representing the command used to run the executable
         *
         * @return `std::string`
         */
        virtual std::string usage() const
        {
            return "";
        }

        /**
         * @brief option descriptions. Use Builder to populate OptionInfo.
         *
         * @return `OptionInfo`
         */
        virtual OptionInfo options() const
        {
            return {};
        }

        /**
         * @brief Method to be executed after parsing of arg options.
         *
         * @param options
         * @return `int`
         */
        virtual int run(const Options& /* options */) const
        {
            return 0;
        }
    };
}
