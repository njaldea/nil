#include <nil/cli/nodes/Help.hpp>

#include <nil/cli/Builder.hpp>
#include <nil/cli/Options.hpp>

namespace nil::cli::nodes
{
    Help::Help(std::ostream& init_os)
        : os(init_os)
    {
    }

    OptionInfo Help::options() const
    {
        return nil::cli::Builder() //
            .flag("help", {.skey = 'h', .msg = "this help"})
            .build();
    }

    int Help::run(const nil::cli::Options& options) const
    {
        options.help(os);
        return 0;
    }
}
