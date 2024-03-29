#include <nil/cli/Node.hpp>

#include <nil/cli/Command.hpp>
#include <nil/cli/OptionInfo.hpp>
#include <nil/cli/Options.hpp>

#include <stdexcept>

namespace nil::cli
{
    Node Node::root()
    {
        return Node{std::make_unique<Command>()};
    }

    Node::Node(std::unique_ptr<Command> init_command)
        : command(std::move(init_command))
    {
    }

    Node::~Node() noexcept = default;

    Node& Node::add(std::string key, std::string description)
    {
        auto instance = std::make_unique<Command>();
        return add(std::move(key), std::move(description), std::move(instance));
    }

    Node& Node::add(std::string key, std::string description, std::unique_ptr<Command> sub_command)
    {
        if (find(key) != nullptr)
        {
            throw std::invalid_argument("[nil][cli][" + key + "] already exists");
        }

        SubNode subnode{
            std::move(key),
            std::move(description),
            std::make_unique<Node>(std::move(sub_command))
        };
        return *(sub.emplace_back(std::move(subnode)).instance);
    }

    int Node::run(int argc, const char* const* argv) const
    {
        if (argc > 1)
        {
            if (const auto* node = find(argv[1]); node != nullptr)
            {
                return node->run(argc - 1, std::next(argv));
            }
        }

        const Options options(command->options(), command->usage(), sub, argc, argv);
        return command->run(options);
    }

    const Node* Node::find(std::string_view name) const
    {
        auto result = std::find_if(
            std::begin(sub),
            std::end(sub),
            [&](const auto& subnode) { return subnode.key == name; }
        );

        if (result != std::end(sub))
        {
            return result->instance.get();
        }
        return nullptr;
    }
}
