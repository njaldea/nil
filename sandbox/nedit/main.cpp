#include "ext/ext.hpp"
#include "gui/gui.hpp"

#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>

#include <array>
#include <iostream>
#include <thread>

// hardcoded assuming that the binary is ran inside `.build` folder
constexpr auto default_file = "../sandbox/nedit/state.dump";

class InOne final: public nil::cli::Command
{
public:
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag("help", {.skey = 'h', .msg = "this help"})
            .number("port", {.skey = 'p', .msg = "use port", .fallback = 1101})
            .param("file", {.skey = 'f', .msg = "file to load", .fallback = default_file})
            .build();
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        const auto port = std::to_string(options.number("port"));
        const auto file = options.param("file");
        std::thread ext(
            [&]()
            {
                const std::array args = {"app_name", "-p", port.c_str()};
                auto root = nil::cli::Node::root<EXT>();
                root.run(args.size(), args.data());
            }
        );

        const std::array args = {"app_name", "-p", port.c_str(), "-f", file.c_str()};
        auto root = nil::cli::Node::root<GUI>();
        root.run(args.size(), args.data());
        ext.join();
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<InOne>();
    root.add<GUI>("gui", "renderer");
    root.add<EXT>("ext", "external");
    return root.run(argc, argv);
}
