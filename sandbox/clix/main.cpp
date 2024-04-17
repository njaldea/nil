#include <nil/clix.hpp>

#include <iostream>

template <int V>
struct Command final: nil::clix::Command
{
    std::string usage() const override
    {
        return " >  <binary> [OPTIONS...]";
    }

    nil::clix::OptionInfo options() const override
    {
        // clang-format off
        return nil::clix::Builder()
            .flag  ("help",   { .skey ='h', .msg = "show this help"                                        })
            .flag  ("spawn",  { .skey ='s', .msg = "spawn"                                                 })
            .number("thread", { .skey ='t', .msg = "number of threads"                                     })
            .number("job",    { .skey ='j', .msg = "number of jobs"    , .fallback = 1     , .implicit = 0 })
            .param ("param",  { .skey ='p', .msg = "default param"     , .fallback = "123"                 })
            .params("mparam", { .skey ='m', .msg = "multiple params"                                       })
            .build();
        // clang-format on
    }

    int run(const nil::clix::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        std::cout                                                     //
            << "flag   -s: " << options.flag("spawn") << std::endl    //
            << "number -t: " << options.number("thread") << std::endl //
            << "number -j: " << options.number("job") << std::endl    //
            << "param  -p: " << options.param("param") << std::endl   //
            << "params -m: " << std::endl;
        for (const auto& item : options.params("mparam"))
        {
            std::cout << " -  " << item << std::endl;
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::clix::Node::root<Command<0>>();
    root.add<Command<1>>("hello", "command for 1:hello") //
        .add<Command<2>>("world", "command for 2:world");
    root.add<Command<3>>("another", "command for 3:another") //
        .add<Command<4>>("dimension", "command for 4:dimension");
    return root.run(argc, argv);
}
