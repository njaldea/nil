import protobufjs from "protobufjs";
import { proto_data } from "./proto_data";

type Binding = { tag: string; valueI64?: number; valueStr?: string; };
type MarkupResponse = { components: string[] };
type BindingResponse = { info: { bindings: Binding[]; }[] };
type FileRequest = { target: string };
type FileResponse = {
    target: string;
    content: string;
};

type ProtoType<T> = {
    encode: (i: T) => { finish: () => Uint8Array };
    decode: (i: Uint8Array) => T;
};

type Proto = {
    MessageType: {
        MessageType_MarkupRequest: 0;
        MessageType_MarkupResponse: 1;
        MessageType_BindingRequest: 2;
        MessageType_BindingResponse: 3;
        MessageType_BindingUpdate: 4;
        MessageType_FileRequest: 5;
        MessageType_FileResponse: 6;
    };

    MarkupResponse: ProtoType<MarkupResponse>;
    FileRequest: ProtoType<FileRequest>;
    FileResponse: ProtoType<FileResponse>;
    BindingResponse: ProtoType<BindingResponse>;
    Binding: ProtoType<Binding>;
};

export const nil_wix_proto = protobufjs.Root.fromJSON(proto_data).lookup(
    "nil.wix.proto"
) as any as Proto;
