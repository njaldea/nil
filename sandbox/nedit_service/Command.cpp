#include "Command.hpp"
#include "Info.hpp"
#include "Service.hpp"

#include <gen/nedit/messages/state.pb.h>

#include <filesystem>
#include <fstream>
#include <iostream>

nil::cli::OptionInfo CMD::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "use port", .fallback = 1101})
        .build();
}

int CMD::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    std::string file = "../sandbox/nedit/state.dump";
    if (!std::filesystem::exists(file))
    {
        return 1;
    }

    nil::nedit::proto::State state;
    std::ifstream fs(file, std::ios::binary);
    state.ParseFromIstream(&fs);

    GraphInfo info = populate(state);

    std::cout << std::flush;

    Service service;
    service.install();
    service.populate(info);
    service.run();

    return 0;
}
