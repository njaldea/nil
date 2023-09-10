#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Builder.hpp"
#include "Command.hpp"
#include "Options.hpp"

namespace nil::cli
{
    class Node final
    {
    public:
        template <typename T>
        static Node root()
        {
            static_assert(std::is_base_of_v<Command, T>, "T must inherit from nil::cli::Command");
            return Node("", std::make_unique<T>());
        }

        Node(std::string name, std::unique_ptr<Command> command);
        ~Node();

        Node(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(const Node&) = delete;
        Node& operator=(Node&&) = delete;

        template <typename T>
        Node& add(std::string key)
        {
            static_assert(std::is_base_of_v<Command, T>, "T must inherit from nil::cli::Command");
            return add(std::move(key), std::make_unique<T>());
        }

        int run(int argc, const char** argv) const;

        std::string name() const;
        std::string description() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;

        Node& add(std::string key, std::unique_ptr<Command> command);
        const Node* find(std::string_view name) const;
    };
}
