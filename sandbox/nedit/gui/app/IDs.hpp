#pragma once

#include <cstdint>
#include <stack>

namespace gui
{
    class IDs final
    {
    public:
        std::uint64_t reserve()
        {
            if (reuse_ids.empty())
            {
                return ++current;
            }
            const auto v = reuse_ids.top();
            reuse_ids.pop();
            return v;
        }

        void release(std::uint64_t id)
        {
            reuse_ids.push(id);
        }

    private:
        std::uint64_t current = 0;
        std::stack<std::uint64_t> reuse_ids;
    };

    class ID final
    {
    public:
        ID(IDs& init_ids)
            : ids(&init_ids)
            , value(ids->reserve())
        {
        }

        ~ID() noexcept
        {
            if (ids)
            {
                ids->release(value);
            }
        }

        ID(ID&& o)
            : ids(o.ids)
            , value(o.value)
        {
            o.ids = nullptr;
        }

        ID(const ID&) = delete;
        ID& operator=(ID&&) = delete;
        ID& operator=(const ID&) = delete;

    private:
        IDs* ids;

    public:
        std::uint64_t value;
    };
}
