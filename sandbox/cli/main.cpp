#include <cli/cli.hpp>
#include <iostream>

template <int V>
struct Command: nil::cli::Command
{
    std::string usage() const override
    {
        return " >  <binary> [OPTIONS...]";
    }

    nil::cli::OptionInfo options() const override
    {
        // clang-format off
        return nil::cli::Builder()
            .flag  ("help",   { .skey ='h', .msg = "show this help"                                        })
            .flag  ("spawn",  { .skey ='s', .msg = "spawn"                                                 })
            .number("thread", { .skey ='t', .msg = "number of threads"                                     })
            .number("job",    { .skey ='j', .msg = "number of jobs"    , .fallback = 1     , .implicit = 0 })
            .param ("param",  { .skey ='p', .msg = "default param"     , .fallback = "123"                 })
            .params("mparam", { .skey ='m', .msg = "multiple params"                                       })
            .build();
        // clang-format on
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << "flag    : " << options.flag("spawn") << std::endl;
        std::cout << "number  : " << options.number("thread") << std::endl;
        std::cout << "number  : " << options.number("job") << std::endl;
        std::cout << "param   : " << options.param("param") << std::endl;
        std::cout << "params  : " << std::endl;
        for (const auto& item : options.params("mparam"))
        {
            std::cout << " -  " << item << std::endl;
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Command<0>>();
    root.add<Command<1>>("hello", "command for 1:hello") //
        .add<Command<2>>("world", "command for 2:world");
    root.add<Command<3>>("another", "command for 3:another") //
        .add<Command<4>>("dimension", "command for 4:dimension");
    return root.run(argc, argv);
}
