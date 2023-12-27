# nil/cli

This library is only intended to simplify arg parsing and allow nesting of sub commands. Internally uses `boost::program_options`.

Simplification is done by limiting the touch points to the internal library and providing more concrete option types with very opinionated defaults.

## Classes

### `nil::cli::Node`

Boost program options does not inherently support nesting of commands. This Node is simply a way to chain the commands.

- initialize a root Node via `nil::cli::Node::root<Command>()`
- add another command to the node via `node.add<AnotherCommand>("key")`
- trigger run to parse arguments and execute the `Command`

### `Command` (inherits from `nil::cli::Command`)

This class is the implementation to be triggered per node. It should inherit from `nil::cli::Command` which provides default and overridable methods.

| method                                    | description                                                          |
| ----------------------------------------- | -------------------------------------------------------------------- |
| `std::string usage() const`               | message to be printed for its own help                               |
| `nil::cli::OptionInfo options() const`    | returns available options for the command. use `nil::cli::Builder`   |
| `int run(const nil::cli::Options&) const` | to be executed after parsing the options                             |

### `nil::cli::Builder`

Builds the objects that will represent each options

NOTE:
- `implicit` is the value when the option is provided without the actual value
    - only `number` has implicit value
- `fallback` is the default value when the option is not provided at all
    - only `number` and `param` has fallback value
    - `param` is required if fallback is not provided

| method                                    | type                       | fallback          | implicit          | required       |
| ----------------------------------------- | -------------------------- | ----------------- | ----------------- | -------------- |
| `Builder& flag(lkey, Flag options)`       | `bool`                     | `false`           | (N/A)             | NO             |
| `Builder& number(lkey, Number options)`   | `int`                      | `0` (overridable) | `1` (overridable) | NO             |
| `Builder& param(lkey, Param  options)`    | `std::string`              |     (overridable) | (N/A)             | if no fallback |
| `Builder& params(lkey, Params options)`   | `std::vector<std::string>` | `[]`              | (N/A)             | NO             |

call `nil::cli::Builder::build()` to retrieve `nil::cli::OptionInfo`.

### `nil::cli::Options`

This class provides a way to access the parsed options. Methods mainly reflects the methods from `nil::cli::Builder`.

| method                                       |
| -------------------------------------------- |
| `bool flag(lkey) const`                      |
| `int number(lkey) const`                     |
| `std::string param(lkey) const`              |
| `std::vector<std::string params(lkey) const` |

## Example

```cpp
#include <nil/cli.hpp>

#include <iostream>

template <int V>
struct Command: nil::cli::Command
{
    std::string usage() const override
    {
        return " >  <binary> [OPTIONS...]";
    }

    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag  ("help",   { .skey ='h', .msg = "show this help"                                        })
            .flag  ("spawn",  { .skey ='s', .msg = "spawn"                                                 })
            .number("thread", { .skey ='t', .msg = "number of threads"                                     })
            .number("job",    { .skey ='j', .msg = "number of jobs"    , .fallback = 1     , .implicit = 0 })
            .param ("param",  { .skey ='p', .msg = "default param"     , .fallback = "123"                 })
            .params("mparam", { .skey ='m', .msg = "multiple params"                                       })
            .build();
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;
        std::cout << "flag    : " << options.flag("spawn") << std::endl;
        std::cout << "number  : " << options.number("thread") << std::endl;
        std::cout << "number  : " << options.number("job") << std::endl;
        std::cout << "param   : " << options.param("param") << std::endl;
        std::cout << "params  : " << std::endl;
        for (const auto& item : options.params("mparam"))
        {
            std::cout << " -  " << item << std::endl;
        }
        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<Command<0>>();
    root.add<Command<1>>("hello", "command for 1:hello") //
        .add<Command<2>>("world", "command for 2:world");
    root.add<Command<3>>("another", "command for 3:another") //
        .add<Command<4>>("dimension", "command for 4:dimension");
    return root.run(argc, argv);
}
```

```
terminal$ ./bin/sandbox_cli hello -h
 >  <binary> [OPTIONS...]                                       //<-- from usage()
OPTIONS:
  -h [ --help ]                     show this help
  -s [ --spawn ]                    spawn
  -t [ --thread ] [=value(=1)] (=0) number of threads
  -j [ --job ] [=value(=0)] (=1)    number of jobs
  -p [ --param ] text (="123")      default param
  -m [ --mparam ] text              multiple params

SUBCOMMANDS:
  world                             command for 2:world
```