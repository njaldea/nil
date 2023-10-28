#include "Link.hpp"
#include "Pin.hpp"

Link::Link(ax::NodeEditor::LinkId init_id, Info info)
    : id(init_id)
    , entry(info.entry)
    , exit(info.exit)
{
}

void Link::render() const
{
    ax::NodeEditor::Link(id, entry->id, exit->id);
}
