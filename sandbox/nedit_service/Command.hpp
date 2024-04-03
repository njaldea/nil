#pragma once

#include <nil/cli.hpp>

struct CMD: nil::cli::Command
{
    nil::cli::OptionInfo options() const override;
    int run(const nil::cli::Options& options) const override;
};
