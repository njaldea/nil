# nil/gate

A node graph library that executes behavior represented by a node. As of the moment, only directed graph is supported (no feedback loop).

## `nil::gate::Core`

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

Lifetime of `Nodes` and `Edges` are all handled by `Core`.
Passing an `Edge` from one `Core` to another `Core` is undefined behavior.

### `nil::gate::ReadOnlyEdge<T>`

This kind of edge is returned by `Core::node<T>(...)`.

Provides an api, `const T& nil::gate::ReadOnlyEdge<T>::value()`, for getting the value owned by the edge.

### `nil::gate::MutableEdge<T>`

This kind of edge comes from the following:
- returned by `Core::edge<T>`.
- returned by batch API.
- accessible from `nil::gate::async_edges`.

Provides an api, `void nil::gate::MutableEdge<T>::set_value(T value)`, for setting a new value for the edge.

`set_value` will only take effect on next `Core::run()` and will not automatically rerun the `Core`.

## Directed Graph

The graph suppored by the library is a directed graph.

The Nodes are executed in proper order only if there is a change in the edge.

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

## Node Signature

```cpp
struct Node
{
    std::tuple<short, long> operator()(bool, char) const;
};

// or

struct Node
{
    std::tuple<short, long> operator()(
        nil::gate::async_edges<float, double>,
        bool,
        char
    ) const;
};
```

A node has the following types:
- Inputs
    - all of the arguments of call operator (minus the async_edges)
    - in example above, `bool` and `char` are inputs of the node

- Sync Outputs
    - all of the types inside the `std::tuple` returned by the call operator
    - if returned type is `void` it is implied to have no output types
    - it is required to either be `void` or `std::tuple`
        - for single return, `std::tuple<T>` is expected
    - in exmaple above, `short` and `long` are the sync outputs of the node

- Async Outputs
    - all of the types inside the first `nil::gate::async_edges<T...>` of call operator
    - if first argument is not `nil::gate::async::edges`, it is implied that there is no async outputs.
    - in exmaple above, `float` and `double` are the sync outputs of the node

## Re-running from changes

`MutableEdge<T>::set_value` does not trigger `Core::run`. This means that it is up to the user to schedule the rerun.

```cpp
int main()
{
    nil::gate::Core core;

    auto [ e ] = core.edge<int>(100);
    core.node<Node>({e});

    // since this is the first time
    // `Node` will be executed with 100 as value held by `e`
    core.run();

    // this will not rerun `Node`
    core.run();

    e->set_value(300);
    
    // `Node` will be executed with 300 as value held by `e`.
    // manual call is required.
    core.run();
}
```

## Batching

Calling multiple `MutableEdge<T>::set_value` for different edges might not result in all edges with the right value.

To batch such changes, `Core::batch(...)` is available

```cpp
int main()
{
    nil::gate::Core core;

    auto [ ei ] = core.edge<int>(100);
    auto [ ef ] = core.edge<float>(200.0);
    core.node<Node>({ei, ef});

    // infinitely run core in another thread.
    std::thread t([&core](){ while (true) { core.run(); }});

    {
        ei->set_value(101);
        ef->set_value(201.0);
        // depending on timing, it is possible that `core.run()`
        // will run with `ei` having `101` and with `ef` having either `200.0` or `201.0`
    }

    // to guarantee batches
    {
        auto batch = core.batch(ei, ef);
        auto& [ bei, bef ] = batch;
        // notice auto&. batch is non-copyable and non-movable.
        // inlining like below should also work.
        // auto [ bei, bef ] = core.batch(ei, ef);
        bei->set_value(102);
        bef->set_value(202.0);
        // this will guarantee that `core.run()`
        // will run with ei having `102` and with `ef` having `202.0`
    }
}
```

## Automatic Batch Commit

`nil::gate::Core` provides a constructor that receives a callback that will be automatically triggered when a batch is finished.

```cpp
// assume this will call the callable in another thread for scheduling
void post(std::function<void()>);

int main()
{
    nil::gate::Core core([](nil::gate::Core& self){ post([&self](){ self.run(); }); });

    auto [ ei ] = core.edge<int>(100);
    auto [ ef ] = core.edge<float>(200.0);
    core.node<Node>({ei, ef});

    {
        ei->set_value(101);
        ef->set_value(201.0);
        core.commit();
    }

    {
        auto [ bei, bef ] = core.batch(ei, ef);
        bei->set_value(101);
        bef->set_value(201.0);
        // will call the callback when the batch goes out of scope.
    }
}
```

This does not affect `set_value` but only batches.

`nil::gate::Core::commit()` is provided to manually call it if there is no batch available (TODO: eval if can be removed).
