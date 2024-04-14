# nil/service

This library is only intended to simplify creation of `tcp`/`udp`/`websocket` client/server.

Simplification is done by abstracting the actual implementation of server/client and simply exposing addition of handlers for specific message types.

## Classes

The classes provided by this library conforms in similar API. available protocols are `tcp` | `udp` | `ws`.

### `nil::service::<protocol>:::Server`

- Options

| name    | protocol    | description                              | default   |
| ------- | ----------- | ---------------------------------------- | --------- |
| port    | tcp/udp/ws  | network port to use                      |           |
| buffer  | tcp/udp/ws  | buffer size to use                       | 1024      |
| timeout | udp         | timeuout to consier a connection is lost | 2 seconds |

### `nil::service::<protocol>:::Client`

- Options

| name    | protocol    | description                              | default   |
| ------- | ----------- | ---------------------------------------- | --------- |
| host    | tcp/udp/ws  | network host to connect to               |           |
| port    | tcp/udp/ws  | network port to connect to               |           |
| buffer  | tcp/udp/ws  | buffer size to use                       | 1024      |
| timeout | udp         | timeuout to consier a connection is lost | 2 seconds |

### methods

| name                             | description                                |
| -------------------------------- | ------------------------------------------ |
| `run()`                          | runs the service                           |
| `stop()`                         | signals to stop the service                |
| `restart()`                      | restart service after stop                 |
| `on_connect(handler)`            | register connect handler                   |
| `on_disconnect(handler)`         | register disconnect handler                |
| `on_message(handler)`            | register message handler                   |
| `send_raw(id, data, size)`       | send message to a specific id/connection   |
| `publish_raw(data, size)`        | sends message to all connection            |
| `send(id, message...)`           | send message to a specific id/connection   |
| `publish(message...)`            | sends message to all connection            |

### `send`/`publish` arguments

- by default only accepts the buffer typed as `std::vector<std::uint8_t>`
- if multiple messages are provided, the messages are serialized one after another
    - the handler should be responsible in parsing these messages
    - see codec for more details

### `codec`

to allow serialization of custom type (message for send/publish), `codec` is expected to be implemented by the user.

the following codecs are already implemented:
- `codec<std::string>`
- `codec<std::uint8_t>`
- `codec<std::uint16_t>`
- `codec<std::uint32_t>`
- `codec<std::uint64_t>`
- `codec<std::int8_t>`
- `codec<std::int16_t>`
- `codec<std::int32_t>`
- `codec<std::int64_t>`

a `codec` is expected to have `serialize` and `deserialize` method. see example below for more information.

```cpp
// codec definition (hpp)

// include this header for base template
#include <nil/service/codec.hpp>

// or include service.hpp as a whole
// #include <nil/service.hpp>

#include <google/protobuf/message_lite.h>

namespace nil::service
{
    template <typename Message>
    constexpr auto is_message_lite = std::is_base_of_v<google::protobuf::MessageLite, Message>;

    template <typename Message>
    struct codec<Message, std::enable_if_t<is_message_lite<Message>>>
    {
        static std::vector<std::uint8_t> serialize(const Message& message)
        {
            return codec<std::string>::serialize(message.SerializeAsString());
        }

        // size is the number of available bytes to read from data
        // size must be adjusted to the right value after consuming portion of the data
        // for example, if the data to deserialize is 100, 100 must be deducted from size
        // > size -= 100;
        static Message deserialize(const void* data, std::uint64_t& size)
        {
            Message m;
            m.ParseFromArray(data, int(size));
            size = 0;
            return m;
        }
    };
}

// consuming cpp

// Include your codec
#include "my_codec.hpp"
// Include service header if `nil/service/codec.hpp` is included in your codec
#include <nil/service.hpp>

template <typename ProtobufMessage>
void foo(nil::service::IService& service, const ProtobufMessage& message)
{
    service.publish(message);
}
```

### `TypedHandler`

This utility handler is provided to identify the message type and trigger which handler to execute

```cpp
void add_handlers(nil::service::IService& service)
{
    service.on_message(
        nil::service::TypedHandler<std::uint32_t>()
            .add(0u, handler_for_0)
            .add(1u, handler_for_1)
    );
}

void send(nil::service::IService& service)
{
    service.publish(0u, "message for 0");
    service.publish(1u, "message for 1");
}
```

## NOTES:
- `restart` is required to be called when `stop`-ed and `run` is about to be called.
- due to the nature of UDP, if one side gets "destroyed" and is able to reconnect "immediately", disconnection will not be "detected".
- (will be fixed in the future) - host is not resolved. currently expects only IP.