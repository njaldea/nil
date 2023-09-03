#include <cli/Node.hpp>

namespace nil::cli
{
    Node::Node(std::unique_ptr<Node::IImpl> impl)
        : mImpl(std::move(impl))
    {
    }

    Node::~Node() = default;

    int Node::run(int argc, const char** argv)
    {
        if (argc > 1)
        {
            if (auto node = find(argv[1]); node != nullptr)
            {
                return node->run(argc - 1, std::next(argv));
            }
        }

        return mImpl->run(argc, argv, mSubNodes);
    }

    std::string Node::name() const
    {
        return mImpl->name();
    }

    std::string Node::description() const
    {
        return mImpl->description();
    }

    Node* Node::find(std::string_view name) const
    {
        auto result = std::find_if(
            std::begin(mSubNodes),
            std::end(mSubNodes),
            [&](const auto& subnode) { return subnode->name() == name; }
        );

        if (result != std::end(mSubNodes))
        {
            return result->get();
        }
        return nullptr;
    }
}
