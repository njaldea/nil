# nil/gate

- [Supported Graph](#supported-graph)
- [nil::gate::Core](#nilgatecore)
    - [Edge](#edge)
        - [edges::ReadOnly](#edgesreadonly)
        - [edges::Mutable](#edgesmutable)
        - [edges::Batch](#edgesbatch)
        - [edges::Compatible](#edgescompatible)
        - [nil::gate::Core::edge](#nilgatecoreedge)
    - [Node](#node)
        - [Input](#input)
        - [Sync Output](#sync-output)
        - [Async Output](#async-output)
        - [Special Arguments](#special-arguments)
        - [nil::gate::Core::node](#nilgatecorenode)
    - [Run](#run)
    - [Batch](#batch)
    - [Commit](#commit)
    - [traits](#traits)
        - [compatibility](#compatibility)
        - [edgify](#edgify)
        - [bias](#bias)
        - [notes](#notes-personal-suggestions)
- [Errors](#errors)

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

Edges are only created through the following:
- returned by `Core::edge`
- output of the Node returned by `Core::node`
- returned by `Core::batch`

### `edges::ReadOnly`

```cpp
#include <nil/gate.hpp>

class Node
{
    std::tuple<float> operator()() const;
};

int main()
{
    nil::gate::Core core;

    auto [ edge ] = core.node();
    //     ┗━━━ nil::gate::edges::ReadOnly<float>*

    // will return the current value.
    // since the node is not yet ran, the edge does not have a value yet.
    // accessing the value of an edge before running the node is undefined behavior.
    edge->value();
}
```

### `edges::Mutable`

This type of edge is also an `edges::ReadOnly`.

NOTE: `set_value` will only take effect on next `Core::run()`.

```cpp
#include <nil/gate.hpp>

int main()
{
    nil::gate::Core core;

    int initial_value = 100;
    auto edge = core.edge(initial_value);
    //   ┗━━━ nil::gate::edges::Mutable<int>*

    edge->value(); // will return 100
    // will request to set the value to 200 on next Core::run()
    edge->set_value(200);
    edge->value(); // will return 100

    core.run();
    edge->value(); // will return 200
}
```

### `edges::Batch`

This type of edge is similar to `edges::Mutable` with the exact same API. These are created when batches are created.

See [Batch](#batch) for more detail.

### `edges::Compatible`

This type of edge is used as a wrapper to `edges::ReadOnly` to be used as an input of a node.

This is intended to be created from `edges::ReadOnly` and is not intended for users to instantiate by themselves.

See [traits](#traits) for more detail how to create compatible edges

### `nil::gate::Core::edge`

Here are the list of available `edge()` signature available to `Core`:

```cpp
// value will be moved to the data inside the edge
nil::gate::edges::Mutable<T>* Core::edge(T value);
```

Requirements of the type for the edge:
- has `operator==`

## Node

A Node is just an object that is Callable (with `operator()`).
Its signature represents the Inputs and Outputs of the Node with some optional special arguments.

NOTE: currently only free function and `T::operator() const` is supported.

### Input

```cpp
#include <nil/gate.hpp>

void free_function_node(float);
//                      ┗━━━ all arguments are treated as an input

int main()
{
    nil::gate::Core core;
    auto edge_f = core.edge(200.f);
    //   ┗━━━ nil::gate::edges::Mutable<float>*

    core.node(&free_function_node, { edge_f });
    //                             ┣━━━ nil::gate::inputs<float>
    //                             ┗━━━ std::tuple<nil::gate::edges::Compatible<float>, ...>;
}
```

### Sync Output

```cpp
#include <nil/gate.hpp>

struct Node
{
    std::tuple<float> operator()() const;
    //         ┗━━━ all tuple types are treated as the sync output
    // returning void is also allowed
    // if non-tuple, it will be treated as one return
};

int main()
{
    nil::gate::Core core;
    
    auto outputs = core.node(Node());
    //   ┣━━━ nil::gate::outputs<float>
    //   ┗━━━ std::tuple<nil::gate::edges::ReadOnly<float>*, ...>;

    {
        auto edge_f = get<0>(outputs);
        //   ┗━━━ nil::gate::edges::ReadOnly<float>*
    }
    {
        auto [ edge_f ] = outputs;
        //     ┗━━━ nil::gate::edges::ReadOnly<float>*
    }
}
```

### Async Output

```cpp
#include <nil/gate.hpp>

struct Node
{
    void operator()(nil::gate::async_output<float> asyncs) const
    //              ┣━━━ this is equivalent to `std::tuple<nil::gate::edges::Mutable<T>*...>`
    //              ┗━━━ this is not treated as an input
    {
        auto [ edge_f ] = asyncs;
        //     ┗━━━ nil::gate::edges::Mutable<float>*
    }
};

int main()
{
    nil::gate::Core core;
    
    nil::gate::outputs<float> = core.node(Node(), { 100.f });
    //                 ┃                          ┗━━━ initial value of the async edges
    //                 ┗━━━ all async outputs is appended to the end of all sync outputs

    {
        auto edge_f = get<0>(outputs);
        //   ┗━━━ nil::gate::edges::ReadOnly<float>*
    }
    {
        auto [ edge_f ] = outputs;
        //     ┗━━━ nil::gate::edges::ReadOnly<float>*
    }
}
```

### Special Arguments

If the 1nd argument is `const nil::gate::Core&`, the `Core` owner will be passed to it.

This can be useful when using async edges.

See [Batch](#batch) section for more detail.

```cpp
#include <nil/gate.hpp>

struct Node
{
    void operator()(const nil::gate::Core& core, nil::gate::async_output<float> asyncs) const
    //              ┃                            ┣━━━ this is equivalent to `std::tuple<edges::Mutable<T>*...>`
    //              ┃                            ┗━━━ this is not treated as an input
    //              ┗━━━ this is not treated as an input
    {
        auto [ edge_f ] = asyncs;
        //     ┗━━━ nil::gate::edges::Mutable<float>*
    }
};

int main()
{
    nil::gate::Core core;

    auto [ edge_f ] = core.node(Node());
    //     ┗━━━ nil::gate::edges::ReadOnly<float>*
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
//  1. `T instance` will be moved (or copied) inside the node
//  2. `std::tuple<A...>` will be used to initialize each async edges
//  3. `nil::gate::inputs<T...>` is simply an alias to `std::tuple<nil::gate::edges::Compatible<T>...>`
//  4. `nil::gate::outputs<T...>` is simply an alias to `std::tuple<nil::gate::edges::ReadOnly<T>*...>`

// no input, no sync output, no async output
void Core::node<T>(T instance);
// no input, has sync output, no async output
nil::gate::outputs<S...> Core::node<T>(T instance);

// has input, not sync output, no async output
void Core::node<T>(T instance, nil::gate::inputs<I...>);
// has input, has sync output, no async output
nil::gate::outputs<S...> Core::node<T>(T instance, nil::gate::inputs<I...>);

// no input, no sync output, has async output
void Core::node<T>(T instance, std::tuple<A...>);
// has input, has sync output, has async output
nil::gate::outputs<S..., A...> Core::node<T>(T instance, std::tuple<A...>, nil::gate::inputs<I...>);
```

## Run

Calling run will execute the nodes in proper order executing the nodes based on their inputs.

Before running all necessary nodes, it will also resolve all calls to `edges::Mutable::set_value` updating the values held by the edges.

NOTE: First call to `Core::run()` will execute all the nodes. Succeeding calls will only execute nodes that expects new data from their inputs.

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
     *  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
     *  ┃       ┏━━━━┓                       ┃
     *  ┃ I1 => ┃    ┃                       ┃
     *  ┃       ┃ N1 ┃          ┏━━━━┓       ┃
     *  ┃ I1 => ┃    ┃ => I3 => ┃    ┃ => I5 ┃
     *  ┃       ┗━━━━┛          ┃ N2 ┃       ┃
     *  ┃ I2 =================> ┃    ┃ => I6 ┃
     *  ┃                       ┗━━━━┛       ┃
     *  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
     */
    auto* edge_i1 = core.edge(10);
    auto* edge_i2 = core.edge(20);
    const auto [ edge_i3, edge_i4 ] = core.node(Node(), { edge_i1, edge_i1 }); // N1
    const auto [ edge_i5, edge_i6 ] = core.node(Node(), { edge_i2, edge_i3 }); // N2

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

    auto ei = core.edge(100);
    //   ┗━━━ nil::gate::edges::Mutable<int>*
    auto ef = core.edge(200.0f);
    //   ┗━━━ nil::gate::edges::Mutable<float>*

    core.node(Node(), { ei, ef });

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
        // auto [ bei, bef ] = core.batch(ei, ef);
        auto batch = core.batch(ei, ef);
        //   ┗━━━ nil::gate::Batch<int, float>
        auto& [ bei, bef ] = batch;
        //  ┃   ┃    ┗━━━ nil::gate::edges::Batch<float>*
        //  ┃   ┗━━━━━━━━ nil::gate::edges::Batch<int>*
        //  ┗━━━ required. batch is non-copyable and non-movable.

        bei->set_value(102);
        bef->set_value(202.0);
        // this will guarantee that `core.run()`
        // will run with ei having `102` and with `ef` having `202.0`
    }
}
```

## Commit

This is a hook that will be called during the following:
- manual call to `Core::commit()`
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

    auto ei = core.edge(100);
    //   ┗━━━ nil::gate::edges::Mutable<int>*
    auto ef = core.edge(200.0);
    //   ┗━━━ nil::gate::edges::Mutable<float>*

    core.node(Node(), { ei, ef });

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
        auto [ bei, bef ] = core.batch({ ei, ef });
        //     ┃    ┃                  ┣━━━ nil::gate::async_outputs<int, float>
        //     ┃    ┃                  ┗━━━ std::tuple<edges::Mutable<int>*, edges::Mutable<float>*>
        //     ┃    ┗━━━ nil::gate::edges::Batch<float>*
        //     ┗━━━━━━━━ nil::gate::edges::Batch<int>*
        bei->set_value(102);
        bef->set_value(202.0);
        // will call the callback when the batch goes out of scope.
    }
}
```

## traits

`nil/gate` provides an overridable traits to allow customization and add rules to graph creation.

### edgify

This trait dictates how the type `T` is going to be interpreted for the edge creation.

The default trait behavior is that `T` provided to edgify will be the same type for the edge type.

This also affects the type evaluation for the sync outputs of a node.

Here is an example of the default behavior:

```cpp
#include <nil/gate.hpp>

int main()
{
    nil::gate::Core core;

    auto edge_i = core.edge(100);
    //   ┗━━━━━━━━ nil::gate::edges::Mutable<int>*

    auto edge_p1 = core.edge(std::unique_ptr<int>());
    //   ┗━━━━━━━━ nil::gate::edges::Mutable<std::unique_ptr<int>>*
    
    auto edge_p2 = core.edge(std::unique_ptr<const int>());
    //   ┗━━━━━━━━ nil::gate::edges::Mutable<std::unique_ptr<const int>>*
}
```

Here is an example of how to override the behavior:

```cpp
#include <nil/gate.hpp>

namespace nil::gate::traits
{
    template <typename T>
    struct edgify<std::unique_ptr<T>>
    {
        using type = std::unique_ptr<const T>;
    };
}

int main()
{
    nil::gate::Core core;

    auto edge_i = core.edge(100);
    //   ┗━━━━━━━━ nil::gate::edges::Mutable<int>*

    auto edge_p1 = core.edge(std::unique_ptr<int>());
    //   ┗━━━━━━━━ nil::gate::edges::Mutable<std::unique_ptr<const int>>*

    auto edge_p2 = core.edge(std::unique_ptr<const int>());
    //   ┗━━━━━━━━ nil::gate::edges::Mutable<std::unique_ptr<const int>>*
}
```

When building a graph, it is ideal that the data owned by the edges is only be modified by the library (not the nodes).
Allowing the node to modify the object referred through indirection will make the graph execution non-deterministic.

This feature is intended for users to opt-in as the library will not be able to cover all of the types that has indirection.

See [biased](#bias) section for more information of `nil/gate`'s suggested rules.

### compatibility

This trait dictates if an edge could be used even if the type of the edge does not match with the expected input edge of the node.

Here is an example of when this is going to be used:

```cpp
#include <nil/gate.hpp>

std::reference_wrapper<const int> switcher(bool flag, int a, int b)
{
    return flag ? a : b;
}

void consumer(int);

int main()
{
    nil::gate::Core core;

    auto* flag = core.edge(false);
    //    ┗━━━━━━━━ nil::gate::edges::Mutable<bool>*
    auto* a = core.edge(1);
    //    ┗━━━━━━━━ nil::gate::edges::Mutable<int>*
    auto* b = core.edge(2);
    //    ┗━━━━━━━━ nil::gate::edges::Mutable<int>*
    const auto [ out ] = core.node(&switcher, {flag, a, b});
    //           ┗━━━━━━━━ nil::gate::edges::ReadOnly<std::reference_wrapper<const int>>*

    core.node(&consumer, { out });
    //                     ┗━━━━━━━━ This will produce a compilation failure.
    //                               ReadOnly<std::reference_wrapper<const int>>
    //                               is not compatible to ReadOnly<int>
}
```

to allow compatibility, users must define how to convert the data by doing the following:

```cpp

#include <nil/gate.hpp>

#include <functional>

namespace nil::gate::traits
{
    template <typename T>
    struct compatibility<
        T
    //  ┣━━━━━━━━ first template type is the type to conver from
    //  ┗━━━━━━━━ this is normally the type expected by the node
        std::reference_wrapper<const T>,
    //  ┗━━━━━━━━ second template type is the type to conver to
    >
    {
        static const T& convert(
            const std::reference_wrapper<const T>& u
        //  ┗━━━━━━━━ this data will be alive through the lifetime of the edge owning it
        )
        {
            return u.get();
        }
    };
}

std::reference_wrapper<const int> switcher(bool flag, int a, int b)
{
    return flag ? a : b;
}

void consumer(int);

int main()
{
    nil::gate::Core core;

    auto* flag = core.edge(false);
    //    ┗━━━━━━━━ nil::gate::edges::Mutable<bool>*
    auto* a = core.edge(1);
    //    ┗━━━━━━━━ nil::gate::edges::Mutable<int>*
    auto* b = core.edge(2);
    //    ┗━━━━━━━━ nil::gate::edges::Mutable<int>*
    const auto [ ref ] = core.node(&switcher, {flag, a, b});
    //           ┗━━━━━━━━ nil::gate::edges::ReadOnly<std::reference_wrapper<const int>>*

    core.node(&consumer, { ref });
    //                     ┗━━━━━━━━ since `compatibility` is defined, this should be accepted by the compiler
}
```

### is_edge_type_valid

This trait is intended for cases where you want to detect and prevent creation of edges that should not be edges.

By default, all of the edge types are valid except for the following:
- T is a pointer type
- T is a reference type
- T is const

```cpp
#include <nil/gate.hpp>

namespace nil::gate::traits
{
    template <typename T>
    struct is_edge_type_valid<std::reference_wrapper<T>>: std::false_type
    {
    };

    template <typename T>
    struct is_edge_type_valid<std::reference_wrapper<const T>>: std::true_type
    {
    };
}
```

With reference to `edgify`, if a type is converted from one type to another, it is a good idea to disable the original type.

See [biased](#bias) section for more information of `nil/gate`'s suggested rules.

### bias

`nil/gate` provides a very opinionated setup from these header:
 - [`#include <nil/gate/bias/compatibility.hpp>`](publish/nil/gate/bias/compatibility.hpp)
     - covers conversion between `T` and `std::reference_wrapper<const T>`
 - [`#include <nil/gate/bias/edgify.hpp>`](publish/nil/gate/bias/edgify.hpp)
     - covers the following types:
         - `std::unique_ptr<T>` to `std::unique_ptr<const T>`
         - `std::shared_ptr<T>` to `std::shared_ptr<const T>`
         - `std::optional<T>` to `std::optional<const T>`
         - `std::reference_wrapper<T>` to `std::reference_wrapper<const T>`
 - [`#include <nil/gate/bias/is_edge_type_valid.hpp>`](publish/nil/gate/bias/is_edge_type_valid.hpp)
     - disables the original types converted bvy `bias/edgify.hpp`
         - `std::unique_ptr<T>`
         - `std::shared_ptr<T>`
         - `std::optional<T>`
         - `std::reference_wrapper<T>`
 - [`#include <nil/gate/bias/nil.hpp>`](publish/nil/gate/bias/nil.hpp)
     - this is a helper header to conform with the author's suggested setup
     - applies all of the author's biases

These are not included from `nil/gate.hpp` and users must opt-in to apply these rules to their graphs.

### NOTES: personal suggestions

- When implementing your own `edgify<T>` traits, avoid converting `T` to something different that is not related to `T`.
    - This will produce weird behavior when defining nodes.
    - `edgify<T>` is mainly intended for things with indirections like pointer-like objects.
- When implementing your own `compatibility` traits, beware of returning a reference type
    - If returning a temporary, you should return an object that will own the data (not a reference).
    - If returning a non-reference type, take note that the conversion will be done everytime the node is triggered.

## Errors

The library tries its best to provide undestandable error messages as much as possible.

The example below is a result of having and invalid `Core` signature. `Core` should always be `const Core&`.

```cpp
float deferred(const nil::gate::Core core, nil::gate::async_outputs<int> z, bool a);
//             ┗━━━━━━━━ this should always be `const nil::gate::Core&`
```

### gcc

```bash
./nil/gate/Core.hpp:50:26: error: use of deleted function ‘nil::gate::errors::Error::Error(nil::gate::errors::Check<false>)’
   50 |             Error core = Check<traits::arg_core::is_valid>();
```

### clang

```bash
./nil/gate/Core.hpp:50:26: fatal error: conversion function from 'Check<traits::arg_core::is_valid>' to 'Error' invokes a deleted function
   50 |             Error core = Check<traits::arg_core::is_valid>();
```

These are the errors detected with similar error message:
 -  any input type is invalid
 -  any sync output type is invalid
 -  any async output type is invalid
 -  async_outputs is invalid
 -  core argument is invalid
 -  input `edges::Readable<T>*` is not compatible to the expected input edge of the node

## TODO

- allow parallelized execution of nodes
- allow a way to disregard node state
    - force execution
- connect two edges together without creating a node
    - this will allow easier feedback loop
