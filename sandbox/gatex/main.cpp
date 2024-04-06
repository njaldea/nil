#include "Command.hpp"

int main(int argc, char** argv)
{
    return nil::cli::Node::root<CMD>().run(argc, argv);
}
