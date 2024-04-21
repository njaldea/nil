#include "ext/ext.hpp"
#include "gui/gui.hpp"

#include <nil/clix.hpp>

#include <array>
#include <iostream>
#include <thread>

// hardcoded assuming that the binary is ran inside `.build` folder
constexpr auto default_file = "../sandbox/nedit/state.dump";

void apply(nil::clix::Node& n)
{
    n.flag("help", {.skey = 'h', .msg = "this help"});
    n.number("port", {.skey = 'p', .msg = "use port", .fallback = 1101});
    n.param("file", {.skey = 'f', .msg = "file to load", .fallback = default_file});
    n.runner(
        [](const nil::clix::Options& options)
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
                    nil::clix::Node node;
                    EXT::apply(node);
                    node.run(args.size(), args.data());
                }
            );

            const std::array args = {"app_name", "-p", port.c_str(), "-f", file.c_str()};
            nil::clix::Node node;
            GUI::apply(node);
            node.run(args.size(), args.data());
            ext.join();
            return 0;
        }
    );
}

int main(int argc, const char** argv)
{
    nil::clix::Node root;
    apply(root);
    root.add("gui", "renderer", GUI::apply);
    root.add("ext", "external", EXT::apply);
    return root.run(argc, argv);
}
