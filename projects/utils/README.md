# nil/utils

A collection of utility classes intended for reuse. Currently provides inspection of Input/Output of call operator.

## Classes

### `nil::utils::traits::types<T, ...>`

A utility type to hold a collection of types. Equivalent to std::tuple without the runtime implementation. Only holds types, not data.

### `nil::utils::traits::callable<T>`

```cpp
struct T
{
    std::tuple<int, char> operator()(float, bool) const;
};

int main()
{
    // nil::utils::traits::types<int, char>(float, bool)
    nil::utils::traits::callable<T>::type;
    
    // nil::utils::traits::types<int, char>
    nil::utils::traits::callable<T>::inputs;
    
    // nil::utils::traits::types<float, bool>
    nil::utils::traits::callable<T>::outputs;
}
```

### `nil::utils::traits::indentity<T>`

A type helper that provides a unique identifier for each type.
Majority of my library will not support RTTI thus, detecting type
is some minimal form is needed.

This is unique for each `T` and inheritance is not taken into account.

```cpp
int main()
{
    const auto v1 = nil::utils::traits::indentity<int>::value;
    const auto v2 = nil::utils::traits::indentity_v<T>;
    // v1 == v2
}
```