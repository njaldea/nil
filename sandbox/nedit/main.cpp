#include "ext/ext.hpp"
#include "gui/gui.hpp"

#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>

#include <iostream>

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<nil::cli::nodes::Help>(std::cout);
    root.add<GUI>("gui", "renderer");
    root.add<EXT>("ext", "external");
    return root.run(argc, argv);
}
