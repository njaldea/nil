#include <cli/Node.hpp>

namespace nil::cli
{
    struct Node::Impl final
    {
        Impl(std::string name, std::unique_ptr<Command> command)
            : name(std::move(name))
            , command(std::move(command))
        {
        }

        int run(int argc, const char** argv, const std::vector<std::unique_ptr<Node>>& subnodes)
        {
            Options options(command->options(), command->usage(), subnodes, argc, argv);
            return command->run(options);
        }

        std::string name;
        std::unique_ptr<Command> command;
        std::vector<std::unique_ptr<Node>> subnodes;
    };

    Node::Node(std::string name, std::unique_ptr<Command> command)
        : mImpl(std::make_unique<Impl>(std::move(name), std::move(command)))
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

        return mImpl->run(argc, argv, mImpl->subnodes);
    }

    std::string Node::name() const
    {
        return mImpl->name;
    }

    std::string Node::description() const
    {
        return mImpl->command->description();
    }

    Node& Node::add(std::string key, std::unique_ptr<Command> command)
    {
        if (find(key) != nullptr)
        {
            throw std::runtime_error("node already exists: " + key);
        }

        mImpl->subnodes.emplace_back(std::make_unique<Node>(std::move(key), std::move(command)));
        return *mImpl->subnodes.back();
    }

    const Node* Node::find(std::string_view name) const
    {
        auto result = std::find_if(
            std::begin(mImpl->subnodes),
            std::end(mImpl->subnodes),
            [&](const auto& subnode) { return subnode->name() == name; }
        );

        if (result != std::end(mImpl->subnodes))
        {
            return result->get();
        }
        return nullptr;
    }
}
