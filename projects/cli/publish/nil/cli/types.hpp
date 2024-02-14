#pragma once

#include <memory>
#include <string>
#include <vector>

namespace nil::cli
{
    class IOption;
    using OptionInfo = std::vector<std::unique_ptr<IOption>>;

    class Node;
    using SubNodes = std::vector<std::tuple<std::string, std::string, std::unique_ptr<Node>>>;
}
