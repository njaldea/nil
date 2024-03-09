#include "IDs.hpp"

#include <utility>

namespace gui
{
    ID::ID(IDs& init_ids, std::uint64_t init_value)
        : ids(&init_ids)
        , value(init_value)
    {
    }

    ID::~ID() noexcept
    {
        if (ids != nullptr)
        {
            ids->release(value);
        }
    }

    ID::ID(ID&& o) noexcept
        : ids(o.ids)
        , value(o.value)
    {
        o.ids = nullptr;
    }

    ID& ID::operator=(ID&& o) noexcept
    {
        ids = std::exchange(o.ids, nullptr);
        value = o.value;
        return *this;
    }

    ID IDs::reserve()
    {
        if (reuse_ids.empty())
        {
            return {*this, ++current};
        }
        const auto v = reuse_ids.top();
        reuse_ids.pop();
        return {*this, v};
    }

    ID IDs::reserve(std::uint64_t id)
    {
        if (id > current)
        {
            current = id + 1;
        }
        return {*this, id};
    }

    void IDs::release(std::uint64_t id)
    {
        reuse_ids.push(id);
    }
}
