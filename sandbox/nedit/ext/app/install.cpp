#include "install.hpp"

#include "Control.hpp"

#include <format>
#include <iostream>

namespace ext
{
    template <typename T>
    struct Input
    {
        std::tuple<T> operator()(const T& value) const
        {
            std::cout << std::format("Input[{}]: <{}>\n", name, value) << std::flush;
            return {value};
        }

        std::string name;
    };

    struct Add
    {
        std::tuple<int> operator()(int l, int r) const
        {
            std::cout << std::format("Add: {} + {} = {}\n", l, r, l + r) << std::flush;
            return {l + r};
        }
    };

    struct Mul
    {
        std::tuple<int> operator()(int l, int r) const
        {
            std::cout << std::format("Mul: {} * {} = {}\n", l, r, l * r) << std::flush;
            return {l * r};
        }
    };

    struct Inverter
    {
        std::tuple<int> operator()(bool l, int r) const
        {
            std::cout << std::format("Inv: {} => {}\n", l ? 'T' : 'F', l ? -r : r) << std::flush;
            return {l ? -r : r};
        }
    };

    template <typename T>
    struct Consume
    {
        void operator()(const T& v) const
        {
            std::cout << std::format("Consume: {}\n", v) << std::flush;
        }
    };

    void install(ext::App& app)
    {
        using text = std::string;
        // clang-format off
        // 1. change to _edge
        // 2.1 consider not returning app. instead, return a builder to add controls to the nodes.
        // 2.2 .add_node<Add>("Add with value", Controls<MinMax<int>, MinMax<int>>{{5, 0 ,10}, {5, 0 ,10}}, ...)
        // 2.3 don't use types for control, instead just support via the builder
        // app.add_node<Inverter>("Inverter", ....)
        //     .slider({value, min, max})
        //     .checkbox({value});
        app.add_pin<bool> ({.label = "bool"  , .color = {{1.0f, 0.0f, 0.0f, 1.0f}}});
        app.add_pin<int>  ({.label = "int"   , .color = {{0.0f, 1.0f, 0.0f, 1.0f}}});
        app.add_pin<float>({.label = "float" , .color = {{0.0f, 0.0f, 1.0f, 1.0f}}});
        app.add_pin<text> ({.label = "string", .color = {{0.0f, 1.0f, 0.5f, 1.0f}}});
        app.add_node<Input<bool> , Value<bool>  >({.label = "Input_b", .controls={{false}}}           , "b");
        app.add_node<Input<int>  , MinMax<int>  >({.label = "Input_i", .controls={{5, 0, 10}}}        , "i");
        app.add_node<Input<float>, MinMax<float>>({.label = "Input_f", .controls={{0.5f, 0.0f, 1.0f}}}, "f");
        app.add_node<Input<text> , Value<text>  >({.label = "Input_s", .controls={{"hello world"}}}   , "s");
        // Enum (combobox) is in WIP state
        // .add_node<Input<text> , Enum         >({.label = "Input_c", .controls={{"1", {"1", "2", "3"}}}}   , "s")
        app.add_node<Inverter>        ({.label = "Inverter"});
        app.add_node<Add>             ({.label = "Add"});
        app.add_node<Add, MinMax<int>>({.label = "Add with value", .controls={{5, 0, 10}}});
        app.add_node<Mul>             ({.label = "Mul"});
        app.add_node<Mul, MinMax<int>>({.label = "Mul with value", .controls={{5, 0, 10}}});
        app.add_node<Consume<bool>>   ({.label = "Consume<b>"});
        app.add_node<Consume<int>>    ({.label = "Consume<i>"});
        app.add_node<Consume<float>>  ({.label = "Consume<f>"});
        app.add_node<Consume<text>>   ({.label = "Consume<s>"});
        // clang-format on

        // for version 2. figure out if i can drag drop scripts(or c++?)
        // as node like game engine UIs.
    }
}
