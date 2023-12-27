#include "install.hpp"

#include "Control.hpp"

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

    void install(ext::App& app)
    {
        using text = std::string;
        // clang-format off
        app //
            .add_pin<bool> ({.label = "bool"  , .color = {1.0f, 0.0f, 0.0f, 1.0f}})
            .add_pin<int>  ({.label = "int"   , .color = {0.0f, 1.0f, 0.0f, 1.0f}})
            .add_pin<float>({.label = "float" , .color = {0.0f, 0.0f, 1.0f, 1.0f}})
            .add_pin<text> ({.label = "string", .color = {0.0f, 1.0f, 0.5f, 1.0f}})
            .add_node<Input<bool> , ext::Value<bool>  >({.label = "Input_b", .controls={{false}}}           , "b")
            .add_node<Input<int>  , ext::MinMax<int>  >({.label = "Input_i", .controls={{5, 0 ,10}}}        , "i")
            .add_node<Input<float>, ext::MinMax<float>>({.label = "Input_f", .controls={{0.5f, 0.0f, 1.0f}}}, "f")
            .add_node<Input<text> , ext::Value<text>  >({.label = "Input_s", .controls={{"hello world"}}}   , "s")
            // Enum (combobox) is in WIP state
            // .add_node<Input<text> , ext::Enum         >({.label = "Input_c", .controls={{"1", {"1", "2", "3"}}}}   , "s")
            .add_node<Inverter>        (Node<Inverter>{.label = "Inverter"})
            .add_node<Add>             ({.label = "Add"})
            .add_node<Add, MinMax<int>>({.label = "Add with value", .controls={{5, 0 ,10}}})
            .add_node<Mul>             ({.label = "Mul"})
            .add_node<Mul, MinMax<int>>({.label = "Mul with value", .controls={{5, 0 ,10}}})
            .add_node<Consume<bool>> ({.label = "Consume<b>"})
            .add_node<Consume<int>>  ({.label = "Consume<i>"})
            .add_node<Consume<float>>({.label = "Consume<f>"})
            .add_node<Consume<text>> ({.label = "Consume<s>"});
        // clang-format on
    }
}
