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

| name                         | description                                |
| ---------------------------- | ------------------------------------------ |
| `run()`                      | runs the service                           |
| `stop()`                     | signals to stop the service                |
| `restart()`                  | restart service after stop                 |
| `on(event, handler)`         | register connect/disconnect handler        |
| `on(type, handler)`          | register message handler                   |
| `send(id, type, data, size)` | send message to a specific id/connection   |
| `publish(type, data, size)`  | sends message to all connection            |

for client, `publish` works the same as `send` to server.

NOTE:
- `restart` is required to be called when `stop`-ed and `run` is about to be called.
- due to the nature of UDP, if one side gets "destroyed" and is able to reconnect "immediately", disconnection will not be "detected".

## NOTES: (will be fixed in the future)
- host is not resolved. currently expects only IP.
