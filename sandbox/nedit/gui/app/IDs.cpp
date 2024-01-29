#include "IDs.hpp"

namespace gui
{
    ID::ID(IDs& init_ids, std::uint64_t init_value)
        : ids(&init_ids)
        , value(init_value)
    {
    }

    ID::~ID() noexcept
    {
        if (ids)
        {
            ids->release(value);
        }
    }

    ID::ID(ID&& o)
        : ids(o.ids)
        , value(o.value)
    {
        o.ids = nullptr;
    }

    ID IDs::reserve()
    {
        if (reuse_ids.empty())
        {
            return ID(*this, ++current);
        }
        const auto v = reuse_ids.top();
        reuse_ids.pop();
        return ID(*this, v);
    }

    ID IDs::reserve(std::uint64_t value)
    {
        if (value > current)
        {
            current = value + 1;
        }
        return ID(*this, value);
    }

    void IDs::release(std::uint64_t id)
    {
        reuse_ids.push(id);
    }
}
