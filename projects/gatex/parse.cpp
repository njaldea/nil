#include <nil/gatex/parse.hpp>

#include <gen/nedit/messages/state.pb.h>

namespace nil::gatex
{
    nil::gatex::GraphInfo parse(const nil::nedit::proto::State& state)
    {
        nil::gatex::GraphInfo info;
        const auto& graph = state.graph();

        for (const auto& node : graph.nodes())
        {
            info.add_node(nil::gatex::NodeInfo{
                .id = node.id(),
                .type = node.type(),
                .inputs = {node.inputs().begin(), node.inputs().end()},
                .outputs = {node.outputs().begin(), node.outputs().end()},
                .controls = {node.controls().begin(), node.controls().end()} //
            });
        }

        for (const auto& link : graph.links())
        {
            info.add_link(nil::gatex::LinkInfo{
                .id = link.id(),
                .type = link.type(),
                .input = link.input(),
                .output = link.output() //
            });
        }

        info.score();
        info.metadata = state.types().SerializeAsString();
        return info;
    }
}
