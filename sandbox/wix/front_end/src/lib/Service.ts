export class Service
{
    constructor({route, host, port, buffer}: { host?: string, route?: string, port?: number, buffer: number})
    {
        this.#id = `${host ?? 'localhost'}${port != null ? `:${port}`: ''}${route ?? '/'}`;
        this.#buffer = buffer;
        this.#state = 0;
        this.#connect = null;
        this.#disconnect = null;
        this.#message = null;
        this.#socket = null;
    }

    on_message(handler: (id: string, data: Uint8Array) => void)
    {
        this.#message = handler;
    }

    on_connect(handler: (id: string) => void)
    {
        this.#connect = handler;
    }

    on_disconnect(handler: (id: string) => void)
    {
        this.#disconnect = handler;
    }

    // non-blocking (run is blocking in c++)
    // calling it again should not do anything
    start()
    {
        if (this.#socket == null && this.#state !== 1)
        {
            this.#socket = new WebSocket(`ws://${this.#id}`);
            this.#socket.binaryType = "arraybuffer";
            this.#socket.onopen = () => {
                this.#state = 1;
                if (this.#connect) {
                    this.#connect(this.#id);
                }
            };
            this.#socket.onclose = () => {
                // happens only when it connected at first
                if (this.#state === 1) {
                    if (this.#disconnect) {
                        this.#disconnect(this.#id);
                    }
                    this.#state = 2;
                    this.#reconnect();
                }
            };
            this.#socket.onerror = () => {
                if (this.#state !== 1)
                {
                    this.#state = 2;
                    this.#reconnect();
                }
            };
            this.#socket.onmessage = (event: MessageEvent) => {
                if (this.#message) {
                    this.#message(this.#id, new Uint8Array(event.data));
                }
            };
        }
    }

    stop()
    {
        this.#state = 0;
        if (this.#socket != null)
        {
            this.#socket.close();
            this.#socket = null;
        }
    }

    publish(data: Uint8Array)
    {
        if (this.#socket != null)
        {
            this.#socket.send(data);
        }
    }

    send(id: string, data: Uint8Array)
    {
        if (id === this.#id)
        {
            this.publish(data);
        }
    }

    #reconnect()
    {
        if (this.#socket != null)
        {
            this.#socket.close();
            this.#socket = null;
        }
        setTimeout(() => this.start(), 25);
    }

    #state; // 0 idle, 1 has connected, 2 keep reconnecting
    #connect: null | ((id: string) => void);
    #disconnect: null | ((id: string) => void);
    #message: null | ((id: string, data: Uint8Array) => void);

    #id;
    #buffer;
    #socket: null | WebSocket;
};