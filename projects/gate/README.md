# nil/gate

A node graph library that executes behavior represented by a node. As of the moment, only directed graph is supported (no feedback loop).

## Classes

### `nil::gate::Core`

An object that owns all of the nodes and edges, and handles the execution of nodes.

To simplify, user is expected to register the nodes (and edges, and call run).

```cpp
struct Node
{
    std::tuple<int, bool> operator(float, char) const;
    std::uint64_t s;
};

int main()
{
    nil::gate::Core core;

    // register external edges
    auto* edge_f = core.edge<float>(200.f);
    // edge_f is a `nil::gate::MutableEdge<float>*`

    auto* edge_c = core.edge<char>('A');
    // edge_c is a `nil::gate::MutableEdge<char>*`

    // 100u is passed forward to Node (via uniform initialization)
    // returned edges are edges intended for users to:
    //  -   pass forward to another edge
    //  -   inspect the value after running
    const auto [ edge_i, edge_b ] = core.node<Node>({ edge_f, edge_c }, 100u);
    // edge_i is a `nil::gate::ReadOnlyEdge<float>*`
    // edge_b is a `nil::gate::ReadOnlyEdge<char>*`

    // this will execute `Node` with input 200.0f and 'A'
    core.run(); // run the graph
    
    if (edge_i->has_value()) {
        edge_i->value();
    }
    if (edge_c->has_value()) {
        edge_c->value();
    }
}
```

Life time of `Nodes` and `Edges` are all handled by `Core`.
Passing an `Edge` from one `Core` to another `Core` is undefined behavior.

### `nil::gate::ReadOnlyEdge<T>`

This kind of edge are returned by `Core::node<T>(...)`.

### `nil::gate::MutableEdge<T>`

This kind of edge are returned by `Core::edge<T>`.
These are the outlier edge intended for users to set value to affect the execution of the graph.

## Graph

The graph suppored by the library is a directed graph. The Nodes are executed in proper order only if there is a change in the edge.

See example below with the following Graph

```cpp
struct Node
{
    std::tuple<int, int> operator(int, int) const;
};

int main()
{
    nil::gate::Core core;

    /**
     *  N1 and N2 are chained
     *  | ---------------------------------- |
     *  |                        ----        |
     *  | I2 =================> | N2 | => I5 |
     *  |        ----           |    | => I6 |
     *  | I1 => | N1 | => I3 => |    |       |
     *  | I1 => |    |           ----        |
     *  |        ----                        |
     *  | ---------------------------------- |
     */
    auto* edge_i1 = core.edge<int>(10);
    auto* edge_i2 = core.edge<int>(20);
    const auto [ edge_i3, edge_i4 ] = core.node<Node>({ edge_i1, edge_i1 }); // N1
    const auto [ edge_i5, edge_i6 ] = core.node<Node>({ edge_i2, edge_i3 }); // N2

    // This will execute N1 then N2
    core.run();

    edge_i2->set_value(3);

    // This will execute only N2
    core.run();
}
```

### TODO
- allow feed back loop
- allow node cancellation
- allow node self mutation