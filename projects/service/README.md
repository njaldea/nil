# nil/service

This library is only intended to simplify creation of `tcp`/`udp`/`websocket` client/server.

Simplification is done by abstracting the actual implementation of server/client and simply exposing addition of handlers for specific message types.

## Classes

The classes provided by this library conforms in similar API. available protocols are `tcp` | `udp` | `ws`.

### `nil::service::<protocol>:::Server`

- Options

| name    | protocol | description                              | default   |
| ------- | -------- | ---------------------------------------- | --------- |
| port    | tcp/udp  | network port to use                      |           |
| buffer  | tcp/udp  | buffer size to use                       | 1024      |
| timeout | udp      | timeuout to consier a connection is lost | 2 seconds |

### `nil::service::<protocol>:::Client`

- Options

| name    | protocol | description                              | default   |
| ------- | -------- | ---------------------------------------- | --------- |
| host    | tcp/udp  | network host to connect to               |           |
| port    | tcp/udp  | network port to connect to               |           |
| buffer  | tcp/udp  | buffer size to use                       | 1024      |
| timeout | udp      | timeuout to consier a connection is lost | 2 seconds |

### methods

| name                         | description                              |
| ---------------------------- | ---------------------------------------- |
| `start()`                    | starts the service                       |
| `stop()`                     | signals to stop the service              |
| `on(event, handler)`         | register connect/disconnect handler      |
| `on(type, handler)`          | register message handler                 |
| `send(id, type, data, size)` | send message to a specific id/connection |
| `publish(type, data, size)`  | sends message to all connection          |

for client, `publish` works the same as `send`` to servier

## NOTES: (will be fixed in the future)
- currently works only in one single thread only (calling start in multiple threads is not expected)
- ws is in WIP state
- id currently is just port. this is not enough
- host is not resolved. currently expects only IP.