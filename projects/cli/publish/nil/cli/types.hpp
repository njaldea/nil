#pragma once

#include "IOption.hpp"

#include <memory>
#include <string>
#include <vector>

namespace nil::cli
{
    using OptionInfo = std::vector<std::unique_ptr<IOption>>;

    class Node;
    using SubNodes = std::vector<std::tuple<std::string, std::string, std::unique_ptr<Node>>>;
}
