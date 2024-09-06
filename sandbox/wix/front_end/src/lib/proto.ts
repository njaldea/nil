import protobufjs from "protobufjs";
import { proto_data } from "./proto_data";

type Range = {
    id: number;
    value: number;
    min: number;
    max: number;
    step: number;
    label: string;
};

type Block = {
    label: string;
    widgets: ({ range: Range } | { text: Text } | { block: Block })[];
};

type WixRoot = { blocks: Block[] };

type I64Update = { id: number; value: number };
type StringUpdate = { id: number; value: string };
type MarkupResponse = { components: string[] };
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
        MessageType_Wix: 0;
        MessageType_I64Update: 1;
        MessageType_StringUpdate: 2;
        MessageType_MarkupRequest: 3;
        MessageType_MarkupResponse: 4;
        MessageType_FileRequest: 5;
        MessageType_FileResponse: 6;
    };

    Wix: ProtoType<WixRoot>;
    I64Update: ProtoType<I64Update>;
    StringUpdate: ProtoType<StringUpdate>;
    MarkupResponse: ProtoType<MarkupResponse>;
    FileRequest: ProtoType<FileRequest>;
    FileResponse: ProtoType<FileResponse>;
};

export const nil_wix_proto = protobufjs.Root.fromJSON(proto_data).lookup(
    "nil.wix.proto"
) as any as Proto;
