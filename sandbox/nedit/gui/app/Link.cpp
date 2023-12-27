#include "Link.hpp"
#include "Pin.hpp"

namespace gui
{
    Link::Link(IDs& init_ids, Info info)
        : id(init_ids)
        , entry(info.entry)
        , exit(info.exit)
    {
    }

    void Link::render() const
    {
        ax::NodeEditor::Link(id.value, entry->id.value, exit->id.value);
    }

}
