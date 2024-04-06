#include <nil/gatex/load.hpp>
#include <nil/gatex/parse.hpp>

#include <gen/nedit/messages/state.pb.h>

#include <fstream>

namespace nil::gatex
{
    GraphInfo load(const std::string& path)
    {
        nil::nedit::proto::State state;
        std::ifstream fs(path, std::ios::binary);
        state.ParseFromIstream(&fs);
        return parse(state);
    }
}
