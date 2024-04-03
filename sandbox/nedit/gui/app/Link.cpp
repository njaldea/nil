#include "Link.hpp"
#include "Pin.hpp"

namespace gui
{
    Link::Link(ID init_id, Info info)
        : id(std::move(init_id))
        , entry(info.entry)
        , exit(info.exit)
    {
    }

    void Link::render() const
    {
        ax::NodeEditor::Link(id.value, entry->id.value, exit->id.value, entry->icon->color);
    }
}
