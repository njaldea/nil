#pragma once

#include <gen/nedit/messages/node_info.pb.h>

namespace ext
{
    template <typename T>
    struct Value
    {
        T value;
    };

    template <typename T>
    struct MinMax
    {
        T value;
        T min;
        T max;
    };

    struct Enum
    {
        std::string value;
        std::vector<std::string> selection;
    };

    void to_message(const Value<bool>& control, nil::nedit::proto::NodeInfo& info);
    void to_message(const MinMax<int>& control, nil::nedit::proto::NodeInfo& info);
    void to_message(const MinMax<float>& control, nil::nedit::proto::NodeInfo& info);
    void to_message(const Value<std::string>& control, nil::nedit::proto::NodeInfo& info);
    void to_message(const Enum& control, nil::nedit::proto::NodeInfo& info);
}
