#pragma once

#include "../Options.hpp"
#include "../types.hpp"

#include <ostream>

namespace nil::cli::nodes
{
    class Help final: public nil::cli::Command
    {
    public:
        Help(std::ostream& init_os);

        OptionInfo options() const;

        int run(const nil::cli::Options& options) const override;

    private:
        std::ostream& os;
    };
}
