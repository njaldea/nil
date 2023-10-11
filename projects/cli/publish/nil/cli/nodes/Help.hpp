#pragma once

#include "../Options.hpp"
#include "../types.hpp"

#include <ostream>

namespace nil::cli::nodes
{
    struct Help final: nil::cli::Command
    {
        Help(std::ostream& init_os)
            : os(init_os)
        {
        }

        int run(const nil::cli::Options& options) const override
        {
            options.help(os);
            return 0;
        }

        std::ostream& os;
    };
}
