#include "Link.hpp"
#include "Pin.hpp"

Link::Link(ax::NodeEditor::LinkId init_id, Pin* init_entry, Pin* init_exit)
    : id(init_id)
    , entry(init_entry)
    , exit(init_exit)
{
}

void Link::render()
{
    ax::NodeEditor::Link(id, entry->id, exit->id);
}
