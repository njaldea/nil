#pragma once

#include <cstdint>
#include <stack>

namespace gui
{
    class IDs;

    class ID final
    {
    private:
        friend class IDs;
        ID(IDs& init_ids, std::uint64_t value);

    public:
        ~ID() noexcept;
        ID(ID&& o) noexcept;
        ID& operator=(ID&& o) noexcept;

        ID(const ID&) = delete;
        ID& operator=(const ID&) = delete;

    private:
        IDs* ids;

    public:
        std::uint64_t value;
    };

    class IDs final
    {
    public:
        ID reserve();
        ID reserve(std::uint64_t id);

    private:
        friend class ID;
        void release(std::uint64_t id);

        std::uint64_t current = 0;
        std::stack<std::uint64_t> reuse_ids;
    };
}
