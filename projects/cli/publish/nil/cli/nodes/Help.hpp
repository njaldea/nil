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

        OptionInfo options() const
        {
            return nil::cli::Builder() //
                .flag("help", {.skey = 'h', .msg = "this help"})
                .build();
        }

        int run(const nil::cli::Options& options) const override
        {
            options.help(os);
            return 0;
        }

        std::ostream& os;
    };
}
