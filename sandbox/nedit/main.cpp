#include "ext/ext.hpp"
#include "gui/gui.hpp"

#include <nil/clix.hpp>

#include <array>
#include <iostream>
#include <nil/clix/node.hpp>
#include <thread>

// hardcoded assuming that the binary is ran inside `.build` folder
constexpr auto default_file = "../sandbox/nedit/state.dump";

void apply(nil::clix::Node& n)
{
    flag(n, "help", {.skey = 'h', .msg = "this help"});
    number(n, "port", {.skey = 'p', .msg = "use port", .fallback = 1101});
    param(n, "file", {.skey = 'f', .msg = "file to load", .fallback = default_file});
    use(n,
        [](const nil::clix::Options& options)
        {
            if (flag(options, "help"))
            {
                help(options, std::cout);
                return 0;
            }
            const auto port = std::to_string(number(options, "port"));
            const auto file = param(options, "file");
            std::thread ext(
                [&]()
                {
                    const std::array args = {"app_name", "-p", port.c_str()};
                    auto node = nil::clix::create_node();
                    EXT::apply(node);
                    run(node, args.size(), args.data());
                }
            );

            const std::array args = {"app_name", "-p", port.c_str(), "-f", file.c_str()};
            auto node = nil::clix::create_node();
            GUI::apply(node);
            run(node, args.size(), args.data());
            ext.join();
            return 0;
        });
}

int main(int argc, const char** argv)
{
    auto root = nil::clix::create_node();
    apply(root);
    sub(root, "gui", "renderer", GUI::apply);
    sub(root, "ext", "external", EXT::apply);
    return run(root, argc, argv);
}
