#include <cli/cli.hpp>
#include <iostream>

template <int V>
struct Command: nil::cli::Command
{
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag("show this help", "help", 'h')
            .flag("spawn", "spawn")
            .counter("threads", "thread", 't')
            .param("param", "param", 'p', "default param")
            .params("mparam", "mparam", 'm')
            .build();
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << options.flag("spawn") << std::endl;
        std::cout << options.counter("thread") << std::endl;
        std::cout << options.param("param") << std::endl;
        for (const auto& item : options.params("mparam"))
        {
            std::cout << item << std::endl;
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Command<0>>();
    root.add<Command<1>>("hello") //
        .add<Command<2>>("wtf");
    root.add<Command<3>>("again");
    return root.run(argc, argv);
}
