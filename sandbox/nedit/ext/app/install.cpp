#include "install.hpp"

#include "Control.hpp"

#include <thread>

namespace ext
{
    template <typename T>
    struct Input
    {
        std::tuple<T> operator()(const T& value) const
        {
            std::cout << "Input[" << name << "]: " << '<' << value << '>' << std::endl;
            return {value};
        }

        std::string name;
    };

    struct Add
    {
        std::tuple<int> operator()(int l, int r) const
        {
            std::cout << l << " + " << r << std::endl;
            return {l + r};
        }
    };

    struct Mul
    {
        std::tuple<int> operator()(int l, int r) const
        {
            std::cout << l << " * " << r << std::endl;
            return {l * r};
        }
    };

    struct Inverter
    {
        std::tuple<int> operator()(bool l, int r) const
        {
            std::cout << l << " ! " << r << std::endl;
            return {l ? -r : r};
        }
    };

    template <typename T>
    struct Consume
    {
        void operator()(const T& v) const
        {
            std::cout << "Consume: " << v << std::endl;
        }
    };

    struct DeferredAdd
    {
        void operator()(std::tuple<nil::gate::MutableEdge<int>*> a, int l, int r) const
        {
            rerun(
                [a, l, r]()
                {
                    get<0>(a)->set_value(l + r);
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            );
        }

        std::function<void(std::function<void()>)> rerun;
    };

    void install(ext::App& app, std::function<void(std::function<void()>)> rerun)
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
        app.add_node<Input<int>  , MinMax<int>  >({.label = "Input_i", .controls={{5, 0 ,10}}}        , "i");
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
        app.add_node<DeferredAdd>     ({.label = "DeferredAdd<i>"}, {0}, rerun);
        // clang-format on

        // for version 2. figure out if i can drag drop scripts(or c++?)
        // as node like game engine UIs.
    }
}
