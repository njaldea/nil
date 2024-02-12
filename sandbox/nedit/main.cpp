#include "ext/ext.hpp"
#include "gui/gui.hpp"

#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>

#include <iostream>
#include <thread>

class InOne final: public nil::cli::Command
{
public:
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag("help", {.skey = 'h', .msg = "this help"})
            // .number("port", {.skey = 'p', .msg = "port", .fallback = 1101})
            .build();
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        const auto args = std::array<const char*, 1>({"app_name"});
        std::thread ext(
            [&]()
            {
                auto root = nil::cli::Node::root<EXT>();
                root.run(args.size(), args.data());
            }
        );

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
