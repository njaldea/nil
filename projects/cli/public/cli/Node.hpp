#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Builder.hpp"
#include "Options.hpp"

namespace nil::cli
{
    class Node final
    {
    private:
        struct IImpl
        {
            virtual std::string name() const = 0;
            virtual std::string description() const = 0;
            virtual int run(
                int argc,
                const char** argv,
                const std::vector<std::unique_ptr<Node>>& subnodes
            ) = 0;
        };

        template <typename T>
        class Impl final: public IImpl
        {
        public:
            Impl(std::string name)
                : mName(std::move(name))
            {
            }

            int run(int argc, const char** argv, const std::vector<std::unique_ptr<Node>>& subnodes)
                override
            {
                Options options(mNode.options(), mNode.usage(), subnodes, argc, argv);
                return mNode.run(options);
            }

            std::string name() const override
            {
                return mName;
            }

            std::string description() const override
            {
                return mNode.description();
            }

        private:
            std::string mName;
            T mNode;
        };

        Node(std::unique_ptr<IImpl> command);

    public:
        template <typename T>
        static Node root()
        {
            return Node(std::make_unique<Impl<T>>(""));
        }

        ~Node();

        Node(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(const Node&) = delete;
        Node& operator=(Node&&) = delete;

        template <typename T>
        Node& add(std::string key)
        {
            if (this->find(key) != nullptr)
            {
                throw std::runtime_error("node not found");
            }

            auto cmd = std::make_unique<Impl<T>>(std::move(key));
            const auto node = new Node(std::move(cmd));
            this->mSubNodes.emplace_back(node);
            return *node;
        }

        int run(int argc, const char** argv);

        std::string name() const;
        std::string description() const;

    private:
        std::unique_ptr<IImpl> mImpl;

        std::vector<std::unique_ptr<Node>> mSubNodes;

        Node* find(std::string_view name) const;
    };
}
