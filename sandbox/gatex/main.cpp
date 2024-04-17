#include "Command.hpp"

int main(int argc, char** argv)
{
    return nil::clix::Node::root<CMD>().run(argc, argv);
}
