# nil/gate

- [Supported Graph](#supported-graph)
- [nil::gate::Core](#nilgatecore)
    - [Edge](#edge)
        - [ReadOnlyEdge](#readonlyedge)
        - [MutableEdge](#mutableedge)
        - [BatchEdge](#batchedge)
        - [nil::gate::Core::edge](#nilgatecoreedge)
    - [Node](#node)
        - [Input](#input)
        - [Sync Output](#sync-output)
        - [Async Output](#async-output)
        - [Special Arguments](#special-arguments)
    - [Batch](#batch)
    - [Commit](#commit)

---

## Supported Graph

**nil/gate** is a library to create a graph with nodes representing a functionality that needs to be executed as the graph is traversed.

Imagine logic gates where edges/connections are not 0/1 (booleans) but can be any customizable types.

**nil/gate** currently only supports **Directed Acyclic Graph**.
It is intended to be simple but easily extensible by creating the proper nodes to support more complex graph.

## `nil::gate::Core`

`nil::gate::Core` is the central object of the library.

It owns and holds all of the nodes and edges registered to it through its API.

## Edge

Edges represents a Single Input - Multiple Output connection.

Requirements of the type for the edge:
- if a smart pointer, it should be pointing to a const
- equal comparable (compatible to `std::equal_to`)

### ReadOnlyEdge

```cpp
#include <nil/gate.hpp>

class Node
{
    std::tuple<float> operator()() const;
};

int main()
{
    nil::gate::Core core;

    // nil::gate::ReadOnlyEdge<float>* 
    auto [edge] = core.node();

    // will return the current value.
    // since the node is not yet ran, the edge does not have a value yet.
    // accessing the value of an edge before running the node is undefined behavior.
    edge->value();
}
```

### MutableEdge

MutableEdge is a ReadableEdge. `set_value` is the only added API to it.

NOTE: `set_value` will only take effect on next `Core::run()`.

```cpp
#include <nil/gate.hpp>

int main()
{
    nil::gate::Core core;

    int initial_value = 100;
    nil::gate::MutableEdge<int>* edge = core.edge<int>(initial_value);

    edge->value(); // will return 100
    // will request to set the value to 200 on next Core::run()
    edge->set_value(200);
    edge->value(); // will return 100

    core.commit();
    edge->value(); // will return 200
}
```

### BatchEdge

BatchEdge are similar to MutableEdge with the exact same API. These are created when batches are created.
See [Batch](#batch) for more detail.

### `nil::gate::Core::edge`

Here are the list of available `edge()` signature available to `Core`

```cpp
// args will be used to instantiate `T` via uniform initialization
nil::gate::MutableEdge<T>* Core::edge(Args... args);

// value will be moved to the data inside the edge
nil::gate::MutableEdge<T>* Core::edge(T value);
```

requirements for `T`:
1. non-const
2. non-pointer and non-reference
3. if a smart pointer (or std::optional), value_type should be `const`
4. compatible to `std::equal_to`

## Node

In simple terms, a Node is just an object that is Callable (with `operator()`). Its signature represents the Inputs and Outputs of the Node.

### Input

```cpp
#include <nil/gate.hpp>

class Node
{
    void operator()(float) const;
    //              ^--- all arguments are treated as an input
};

int main()
{
    nil::gate::Core core;
    nil::gate::MutableEdge<float>* edge_f = core.edge<float>(200.f);

    nil::gate::inputs<float> inputs = { edge_f };
    core.node<Node>({ edge_f });
    //              ^--- nil::gate::inputs<float>
    //              ^--- std::tuple<nil::gate::ReadOnlyEdge<float>*, ...>;
}
```

### Sync Output

```cpp
#include <nil/gate.hpp>

struct Node
{
    std::tuple<float> operator()() const;
    //         ^--- all tuple types are treated as the sync output
    // returning void is also allowed
};

int main()
{
    nil::gate::Core core;
    
    nil::gate::outputs<float> outputs = core.node<Node>();
    // std::tuple<nil::gate::ReadOnlyEdge<float>*, ...>;

    {
        nil::gate::ReadOnlyEdge<float>* edge_f = get<0>(outputs);
    }
    {
        auto [ edge_f ] = outputs;
        //     ^--- nil::gate::ReadOnlyEdge<float>*
    }
}
```

### Async Output

```cpp
#include <nil/gate.hpp>

struct Node
{
    void operator()(nil::gate::async_output<float> asyncs) const
    //              ^--- this is equivalent to `std::tuple<nil::gate::MutableEdge<T>*...>`
    //              ^--- this is not treated as an input
    {
        auto [ edge_f ] = asyncs;
        //     ^--- nil::gate::MutableEdge<float>*
    }
};

int main()
{
    nil::gate::Core core;
    
    nil::gate::outputs<float> = core.node<Node>(std::tuple<float>(100.f));
    //                 ^                        ^--- initial value of the async edges
    //                 ^--- all async outputs is appended to the end of all sync outputs

    {
        nil::gate::ReadOnlyEdge<float>* edge_f = get<0>(outputs);
    }
    {
        auto [ edge_f ] = outputs;
        //     ^--- nil::gate::ReadOnlyEdge<float>*
    }
}
```

### Special Arguments

When using Async Outputs, you might want to batch the updates to the edges.
Batching is a sole responsibility of `nil::gate::Core`.

If the 2nd argument is `const nil::gate::Core&`, the `Core` owner will be passed to it.

See [Batch](#batch) section for more detail.

```cpp
#include <nil/gate.hpp>

struct Node
{
    void operator()(nil::gate::async_output<float> asyncs, const nil::gate::Core& core) const
    //              ^                                      ^--- this is not treated as an input
    //              ^--- this is equivalent to `std::tuple<MutableEdge<T>*...>`
    //              ^--- this is not treated as an input
    {
        auto [ edge_f ] = asyncs;
        //     ^--- nil::gate::MutableEdge<float>*
    }
};

int main()
{
    nil::gate::Core core;
    
    auto [ edge_f ] = core.node<Node>();
    //     ^--- nil::gate::ReadOnlyEdge<float>*
}
```

### `nil::gate::Core::node`

Here are the list of available `node()` signature available to `Core`

```cpp
//  Legend:
//   -  I -- Input
//   -  S -- Sync Output
//   -  A -- Async Output
//
// NOTES:
//  Args are going to be used to instantiate the node using uniform initialization
//  `std::tuple<A...>` will be used to initialize each async edges
//  `nil::gate::inputs<T...>` is simply an alias to `std::tuple<nil::gate::ReadOnlyEdge<T>*...>`
//  `nil::gate::outputs<T...>` is simply an alias to `std::tuple<nil::gate::ReadOnlyEdge<T>*...>`

// no input, no sync output, no async output
void Core::node(Args... args);
// no input, has sync output, no async output
nil::gate::outputs<S...> Core::node(Args... args);

// has input, not sync output, no async output
void Core::node(nil::gate::inputs<I...>, Args... args);
// has input, has sync output, no async output
nil::gate::outputs<S...> Core::node(nil::gate::inputs<I...>, Args... args);

// no input, no sync output, has async output
nil::gate::outputs<A...> Core::node(std::tuple<A...>, Args... args);
// has input, has sync output, has async output
nil::gate::outputs<S..., A...> Core::node(std::tuple<A...>, nil::gate::inputs<I...>, Args... args);
```

## Run

Calling run will execute the nodes in proper order executing the nodes based on their inputs.

Before running all necessary nodes, it will also resolve all calls to `MutableEdge::set_value` updating the values held by the edges to the latest values.

NOTE: first call to `Core::run()` will execute all the nodes. succeeding calls will only execute nodes that expects new data from their inputs.

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
     *  |        ----                        |
     *  | I1 => |    |                       |
     *  |       | N1 |           ----        |
     *  | I1 => |    | => I3 => |    |       |
     *  |        ----           | N2 | => I5 |
     *  | I2 =================> |    | => I6 |
     *  |                        ----        |
     *  | ---------------------------------- |
     */
    auto* edge_i1 = core.edge<int>(10);
    auto* edge_i2 = core.edge<int>(20);
    const auto [ edge_i3, edge_i4 ] = core.node<Node>({ edge_i1, edge_i1 }); // N1
    const auto [ edge_i5, edge_i6 ] = core.node<Node>({ edge_i2, edge_i3 }); // N2

    // This will execute N1 then N2
    core.run();

    edge_i2->set_value(3);

    // Will return 20 since set_value will only be resolved before Core::run()
    edge_i2->value();

    // This will execute only N2
    core.run();

    // Will return 3
    edge_i2->value();
}
```

## Batch

Imagine calling `set_value` and another thread is constantly calling `Core::run`.
If somehow `Core::run` is called in between the calls to multiple `set_value`, that frame might not contain the "right" values for the edges.

We can relate this to database session.

```cpp
#include <nil/gate.hpp>
#include <thread>

struct Node
{
    void operator()(int, float) const;
};

int main()
{
    nil::gate::Core core;

    nil::gate::MutableEdge<int>* ei = core.edge<int>(100);
    nil::gate::MutableEdge<float>* ef = core.edge<float>(200.0);

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
        nil::gate::Batch<int, float> batch = core.batch(ei, ef);

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

## Commit

This is a hook that will be called during the following:
- call to `Core::commit()`
- destruction of a `Batch<T...>`

```cpp
#include <nil/gate.hpp>
#include <functional>

struct Node
{
    void operator()(int, float) const;
};

// assume this will call the callable in another thread for scheduling
void post(std::function<void()>);

void commit_hook(nil::gate::Core& core)
{
    post([&core](){ self.run(); });
}

int main()
{
    nil::gate::Core core;

    core.set_commit(commit_hook);

    nil::gate::MutableEdge<int>* ei = core.edge<int>(100);
    nil::gate::MutableEdge<float>* ef = core.edge<float>(200.0);

    core.node<Node>({ei, ef});

    {
        ei->set_value(101);
        ef->set_value(201.0);
        core.commit(); // manual commit call
    }

    {
        auto [ bei, bef ] = core.batch(ei, ef);
        bei->set_value(102);
        bef->set_value(202.0);
        // will call the callback when the batch goes out of scope.
    }
    
    // also supports receiving tuple of mutable edges
    {
        nil::gate::async_outputs<int, float> edges(ei, ef);
        // ^--- identical to `std::tuple<MutableEdge<int>*, MutableEdge<float>*>
        auto [ bei, bef ] = core.batch(edges);
        bei->set_value(102);
        bef->set_value(202.0);
        // will call the callback when the batch goes out of scope.
    }
}
```