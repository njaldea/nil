#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace nil::cli
{
    struct IOption
    {
        struct Impl;
        virtual ~IOption() = default;
        virtual void apply(const Impl& impl) const = 0;
    };

    using OptionInfo = std::vector<std::unique_ptr<IOption>>;

    class Node;
    using SubNodes = std::vector<std::tuple<std::string, std::string, std::unique_ptr<Node>>>;

    class Options;

    /**
     * @brief Class that user's have to inherit from to enforce
     *  API and provide default behavior.
     */
    struct Command
    {
        virtual ~Command() = default;

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
