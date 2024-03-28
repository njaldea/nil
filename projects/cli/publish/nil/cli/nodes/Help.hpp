#pragma once

#include "../Command.hpp"

#include <ostream>

namespace nil::cli::nodes
{
    class Options;

    class Help final: public nil::cli::Command
    {
    public:
        explicit Help(std::ostream& init_os);
        ~Help() noexcept override = default;

        Help(Help&&) noexcept = delete;
        Help& operator=(Help&&) noexcept = delete;

        Help(const Help&) = delete;
        Help& operator=(const Help&) = delete;

        OptionInfo options() const override;

        int run(const nil::cli::Options& options) const override;

    private:
        std::ostream& os;
    };
}
