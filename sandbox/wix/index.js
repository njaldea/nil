export class Service
{
    constructor({host, port, buffer})
    {
        this.#id = `${host ?? 'localhost'}:${port}`;
        this.#buffer = buffer;
        this.#state = 0;
    }

    on_message(handler)
    {
        this.#message = handler;
    }

    on_connect(handler)
    {
        this.#connect = handler;
    }

    on_disconnect(handler)
    {
        this.#disconnect = handler;
    }

    // non-blocking (run is blocking in c++)
    // calling it again should not do anything
    start()
    {
        if (this.#socket == null)
        {
            this.#socket = new WebSocket(`ws://${this.#id}`);
            this.#socket.addEventListener("open", (event) => {
                this.#state = 1;
                if (this.#connect) {
                    this.#connect(this.#id);
                }
            });
            this.#socket.addEventListener("close", (event) => {
                // happens only when it connected at first
                if (this.#state === 1) {
                    if (this.#disconnect) {
                        this.#disconnect(this.#id);
                    }
                    this.#reconnect();
                }
            });
            this.#socket.addEventListener("error", (event) => {
                // happens when it didn't even connect in the first place
                if (this.#state === 0 && this.#socket != null) {
                    this.#reconnect();
                }
            });
            this.#socket.addEventListener("message", (event) => {
                if (this.#message) {
                    this.#message(this.#id, (new TextEncoder()).encode(event.data));
                }
            });
        }
    }

    stop()
    {
        if (this.#state === 1 && this.#socket)
        {
            this.#socket.close();
        }
        
        this.#state = 0;
        this.#socket = null;
    }

    publish(data)
    {
        if (this.#socket != null)
        {
            this.#socket(data);
        }
    }

    send(id, data)
    {
        if (id === this.#id)
        {
            this.publish(data);
        }
    }

    #reconnect()
    {
        this.#socket = null;
        setTimeout(() => this.start(), 25);
    }

    #state; // 0 idle, 1 keep connected
    #connect;
    #disconnect;
    #message;

    #id;
    #buffer;
    #socket;
};