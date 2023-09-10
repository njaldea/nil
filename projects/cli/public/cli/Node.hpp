#pragma once

#include "Builder.hpp"
#include "types.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace nil::cli
{
    class Node final
    {
    public:
        template <typename T>
        static Node root()
        {
            static_assert(std::is_base_of_v<Command, T>, "T must inherit from nil::cli::Command");
            return Node(std::make_unique<T>());
        }

        Node(std::unique_ptr<Command> command);
        ~Node();

        Node(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(const Node&) = delete;
        Node& operator=(Node&&) = delete;

        template <typename T>
        Node& add(std::string key, std::string description)
        {
            static_assert(std::is_base_of_v<Command, T>, "T must inherit from nil::cli::Command");
            return add(std::move(key), std::move(description), std::make_unique<T>());
        }

        int run(int argc, const char** argv) const;

    private:
        std::unique_ptr<Command> mCommand;
        SubNodes mSubNodes;

        Node& add(std::string key, std::string description, std::unique_ptr<Command> command);
        const Node* find(std::string_view name) const;
    };
}
