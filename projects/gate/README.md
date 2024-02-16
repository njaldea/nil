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
    
    edge_i->value();
    edge_c->value();
}
```

Life time of `Nodes` and `Edges` are all handled by `Core`.
Passing an `Edge` from one `Core` to another `Core` is undefined behavior.

### `nil::gate::ReadOnlyEdge<T>`

This kind of edge is returned by `Core::node<T>(...)`.
Provides an api, `const T& nil::gate::ReadOnlyEdge<T>::value()`, for getting the value owned by the edge.

### `nil::gate::MutableEdge<T>`

This kind of edge comes from the following:
- returned by `Core::edge<T>`.
- received by the Node if the first argument of the node is `std::tuple<MutableEdge<T>*, ...>`

Provides an api, `void nil::gate::MutableEdge<T>::set_value(T value)`, for setting a new value for the edge.
`set_value` will only take effect on next `Core::run()` and will not automatically rerun the `Core`.

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
     *  | I2 =================> |    | => I5 |
     *  |        ----           | N2 | => I6 |
     *  | I1 => |    | => I3 => |    |       |
     *  |       | N1 |           ----        |
     *  | I1 => |    |                       |
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

## Node

A functor that can be registered to `nil/gate` through `nil::gate::Core`.

`Node::operator()` dictates the input/output edges of the node.

```cpp
struct Node
{
    void operator()(bool, char) const;
};

struct Node_With_Return
{
    std::tuple<float, double> operator()(bool, char) const;
};

struct Node_With_Deferred_Output
{
    void operator()(
        std::tuple<
            nil::gate::MutableEdge<float>*,
            nil::gate::MutableEdge<double>*
        >,
        bool,
        char
    ) const;
};


struct Node_With_Deferred_Output_And_Return
{
    std::tuple<short, long> operator()(
        std::tuple<
            nil::gate::MutableEdge<float>*,
            nil::gate::MutableEdge<double>*
        >,
        bool,
        char
    ) const;
};

int main()
{
    nil::gate::Core core;

    auto* edge_bool = core.edge(false); // MutableEdge<bool>*
    auto* edge_char = core.edge('a');   // MutableEdge<char>*

    core.node<Node>({edge_bool, edge_char});
    {
        const auto [
            edge_float, // ReadOnlyEdge<float>*
            edge_double // ReadOnlyEdge<double>*
        ] = core.node<Node_With_Return>(
            {edge_bool, edge_char}  // input edges
        );
    }
    {
        const auto [
            edge_float, // ReadOnlyEdge<float>*
            edge_double // ReadOnlyEdge<double>*
        ] = core.node<Node_With_Deferred_Output>(
            {10.0f, 20.0},          //  std::tuple<float, double>
                                    //   -  these are initializers for the deferred/async edges
            {edge_bool, edge_char}  // input edges
        );
    }
    {
        const auto [
            // these are the return edges
            edge_short, // ReadOnlyEdge<short>*
            edge_long,  // ReadOnlyEdge<long>*
            // these are the deferred/async edges
            edge_float, // ReadOnlyEdge<float>*
            edge_double // ReadOnlyEdge<double>*
        ] = core.node<Node_With_Deferred_Output_And_Return>(
            {10.0f, 20.0},          //  std::tuple<float, double>
                                    //   -  these are initializers for the deferred/async edges
            {edge_bool, edge_char}  // input edges
        );
    }
}
```

### TODO
- include documentation about nodes with async/deferred edges.