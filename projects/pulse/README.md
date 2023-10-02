# nil/pulse

This library is a naive implementation of reactive objects.

Inspired mainly by svelte stores (in javascript)

## Classes

### `nil::pulse::Data<T, ThreadSafety = nil::pulse::NoMutex>`

Due to possible lifetime issues, Data<T> is implemented with `std::shared_ptr` in mind.

| method name                 | description                                                              |
| --------------------------- | ------------------------------------------------------------------------ |
| `create(T value)`           | creates a shared_ptr of Data<T>                                          |
| `get() const`               | returns the current value held by Data<T>                                |
| `set(T value)`              | updates the current value held by the object and invokes all subscribers |
| `subscribe(callback, mode)` | register a callback and returns an unsubscriber for cleanup              |

### mode

Registering a callback as subscriber returns an unsubscriber which is intended to be used to unregister the callback.
`mode` is used to configure the unsubscriber's behavior during its destructor.
By default, `Strong` mode is used which will automatically unregister during destructor.
`Weak` mode is provided to relax it and will only unregister when the unsubscriber is invoked.

### `ThreadSafety`

By default (`nil::pulse::NoMutex`), Data<T> will not do anything for threadsafety.
If it is forseen that threadsafety might be needed (for unsubscribing), 2nd template argument is available for overriding.
`ThreadSafety` requires a type compatible with `std::unique_lock`.

NOTE:
- be careful of memory leak due to cyclic dependency when registering callbacks that captures objects (shared_ptrs)
