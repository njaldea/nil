#include <cli/Node.hpp>

#include <cli/Options.hpp>
#include <cli/types.hpp>

namespace nil::cli
{
    Node::Node(std::unique_ptr<Command> command)
        : mCommand(std::move(command))
    {
    }

    Node::~Node() = default;

    int Node::run(int argc, const char** argv) const
    {
        if (argc > 1)
        {
            if (auto node = find(argv[1]); node != nullptr)
            {
                return node->run(argc - 1, std::next(argv));
            }
        }

        Options options(mCommand->options(), mCommand->usage(), mSubNodes, argc, argv);
        return mCommand->run(options);
    }

    Node& Node::add(std::string key, std::string description, std::unique_ptr<Command> command)
    {
        if (find(key) != nullptr)
        {
            throw std::runtime_error("node already exists: " + key);
        }

        mSubNodes.emplace_back(std::make_tuple(
            std::move(key),
            std::move(description),
            std::make_unique<Node>(std::move(command))
        ));
        return *std::get<2>(mSubNodes.back());
    }

    const Node* Node::find(std::string_view name) const
    {
        auto result = std::find_if(
            std::begin(mSubNodes),
            std::end(mSubNodes),
            [&](const auto& subnode) { return std::get<0>(subnode) == name; }
        );

        if (result != std::end(mSubNodes))
        {
            return std::get<2>(*result).get();
        }
        return nullptr;
    }
}
