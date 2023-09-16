#pragma once

#include "types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace nil::cli
{
    class Node;

    class Options final
    {
        friend class Node;
        Options(
            const OptionInfo& info,
            std::string usage,
            const SubNodes& subnodes,
            int argc,
            const char** argv
        );
        ~Options();

    public:
        /**
         * @brief Print help message to ostream.
         *
         * @param os
         */
        void help(std::ostream& os) const;

        /**
         * @brief Access the value from parsed arguments.
         *  Expected to be used for options registered using Builder::flag
         *
         * @param lkey          registered long key
         * @return bool
         */
        bool flag(const std::string& lkey) const;

        /**
         * @brief Access the value from parsed arguments.
         *  Expected to be used for options registered using Builder::number
         *
         * @param lkey          registered long key
         * @return int
         */
        int number(const std::string& lkey) const;

        /**
         * @brief Access the value from parsed arguments.
         *  Expected to be used for options registered using Builder::param
         *
         * @param lkey          registered long key
         * @return std::string
         */
        std::string param(const std::string& lkey) const;

        /**
         * @brief Access the value from parsed arguments.
         *  Expected to be used for options registered using Builder::params
         *
         * @param lkey                      registered long key
         * @return std::vector<std::string>
         */
        std::vector<std::string> params(const std::string& lkey) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
