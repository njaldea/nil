#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace nil::cli
{
    struct Command;

    class Node;

    struct SubNode
    {
        std::string key;
        std::string description;
        std::unique_ptr<Node> instance;
    };

    class Node final
    {
    public:
        /**
         * @brief create a Node with base nil::cli::Command
         *
         * @return `Node`
         */
        static Node root();

        /**
         * @brief create a Node with custom Command
         *
         * @tparam T    custom command inheriting from nil::cli::Command
         * @tparam Args constructor arguments of T
         * @param args  constructor arguments to be passed to T
         *
         * @return `Node`
         */
        template <typename T, typename... Args>
        static Node root(Args&&... args)
        {
            static_assert(std::is_base_of_v<Command, T>, "T must inherit from nil::cli::Command");
            return Node(std::make_unique<T>(std::forward<Args>(args)...));
        }

        /**
         * @brief Construct a new Node object
         *
         * @param command   Command instance
         */
        explicit Node(std::unique_ptr<Command> command);
        ~Node() noexcept;

        Node(Node&&) noexcept = delete;
        Node& operator=(Node&&) noexcept = delete;

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        /**
         * @brief add nil::cli::Command as subnode
         *
         * @param key           subcommand
         * @param description   message to be used when displaying help]
         *
         * @return `Node&`      created Node
         */
        Node& add(std::string key, std::string description);

        /**
         * @brief add a custom Command
         *
         * @tparam T            custom command inheriting from nil::cli::Command
         * @tparam Args         constructor arguments of T
         * @param key           subcommand
         * @param description   message to be used when displaying help
         * @param args          constructor arguments to be passed to T
         *
         * @return `Node&`      created Node
         */
        template <typename T, typename... Args>
        Node& add(std::string key, std::string description, Args&&... args)
        {
            static_assert(std::is_base_of_v<Command, T>, "T must inherit from nil::cli::Command");
            auto instance = std::make_unique<T>(std::forward<Args>(args)...);
            return add(std::move(key), std::move(description), std::move(instance));
        }

        /**
         * @brief parse and execute the arguments
         *
         * @param argc      1 minus the number of items in argv.
         * @param argv      at least 2 { PROGRAM_NAME, SENTINEL, ...REST }
         *
         * @return `int`    STATUS CODE
         */
        int run(int argc, const char* const* argv) const;

    private:
        std::unique_ptr<Command> command;

        std::vector<SubNode> sub;

        Node& add(std::string key, std::string description, std::unique_ptr<Command> command);
        const Node* find(std::string_view name) const;
    };
}
