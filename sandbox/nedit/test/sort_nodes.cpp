#include "../ext/app/sort_nodes.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(sort_nodes, sort_by_score_already_sorted)
{
    const auto nodes //
        = []()
    {
        std::vector<ext::NodeData> retval;
        retval.emplace_back( //
            ext::NodeData{
                .type = 1,
                .inputs = {2},
                .outputs = {3},
                .controls = {4} //
            }
        );
        retval.emplace_back( //
            ext::NodeData{
                .type = 2,
                .inputs = {3},
                .outputs = {5},
                .controls = {6} //
            }
        );
        retval.emplace_back( //
            ext::NodeData{
                .type = 1,
                .inputs = {5},
                .outputs = {7},
                .controls = {8} //
            }
        );
        return retval;
    }();

    const auto result = ext::sort_by_score(nodes);
    ASSERT_EQ(result.size(), nodes.size());
    for (auto i = 0u; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i].type, nodes[i].type);
        ASSERT_EQ(result[i].inputs, nodes[i].inputs);
        ASSERT_EQ(result[i].outputs, nodes[i].outputs);
        ASSERT_EQ(result[i].controls, nodes[i].controls);
    }
}

TEST(sort_nodes, sort_by_score_reverse_sorted)
{
    const auto nodes //
        = []()
    {
        std::vector<ext::NodeData> retval;
        retval.emplace_back( //
            ext::NodeData{
                .type = 1,
                .inputs = {5},
                .outputs = {7},
                .controls = {8} //
            }
        );
        retval.emplace_back( //
            ext::NodeData{
                .type = 2,
                .inputs = {3},
                .outputs = {5},
                .controls = {6} //
            }
        );
        retval.emplace_back( //
            ext::NodeData{
                .type = 1,
                .inputs = {2},
                .outputs = {3},
                .controls = {4} //
            }
        );
        return retval;
    }();

    const auto result = ext::sort_by_score(nodes);
    ASSERT_EQ(result.size(), nodes.size());
    for (auto i = 0u; i < result.size(); ++i)
    {
        ASSERT_EQ(result[i].type, nodes[result.size() - i - 1].type);
        ASSERT_EQ(result[i].inputs, nodes[result.size() - i - 1].inputs);
        ASSERT_EQ(result[i].outputs, nodes[result.size() - i - 1].outputs);
        ASSERT_EQ(result[i].controls, nodes[result.size() - i - 1].controls);
    }
}
