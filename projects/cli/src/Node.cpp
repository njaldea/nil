#include <nil/cli/Node.hpp>

#include <nil/cli/Options.hpp>
#include <nil/cli/types.hpp>

namespace nil::cli
{
    Node::Node(std::unique_ptr<Command> init_command)
        : command(std::move(init_command))
    {
    }

    Node::~Node() = default;

    int Node::run(int argc, const char** argv) const
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

    Node& Node::add(std::string key, std::string description, std::unique_ptr<Command> sub_command)
    {
        if (find(key) != nullptr)
        {
            throw std::invalid_argument("[nil][cli][" + key + "] already exists");
        }

        sub.emplace_back(std::make_tuple(
            std::move(key),
            std::move(description),
            std::make_unique<Node>(std::move(sub_command))
        ));
        return *std::get<2>(sub.back());
    }

    const Node* Node::find(std::string_view name) const
    {
        auto result = std::find_if(
            std::begin(sub),
            std::end(sub),
            [&](const auto& subnode) { return std::get<0>(subnode) == name; }
        );

        if (result != std::end(sub))
        {
            return std::get<2>(*result).get();
        }
        return nullptr;
    }
}
