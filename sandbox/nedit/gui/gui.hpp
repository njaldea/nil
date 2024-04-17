#pragma once

#include <nil/clix.hpp>

struct GUI: nil::clix::Command
{
    nil::clix::OptionInfo options() const override;

    int run(const nil::clix::Options& options) const override;

    void play() const;
};
