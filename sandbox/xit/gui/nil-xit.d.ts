export type Action<T> = (target: T) => {
    destroy: () => void;
};

export type Writable<T> = {
    set: (v: T) => void;
    subscribe: (cb: (v: T) => void) => () => void;
    update: (cb: (v: T) => T) => void;
}

export type CoDec = {
    encode: (o: object) => Uint8Array;
    decode: (a: Uint8Array) => object;
};

export type Loader = {
    one: (f: string) => Action<HTMLDivElement>;
    all: (f: string[]) => Action<HTMLDivElement>;
};

export type Xit = {
    binding: {
        string: (t: string, v: string) => Writable<string>;
        number: (t: string, v: number) => Writable<number>;
        buffer: (t: string, v: Uint8Array) => Writable<Uint8Array>;
        json: (t: string, v: object, codec: CoDec) => Writable<object>;
    };
    loader: Loader;
};
export declare const json_string_codec: CoDec;