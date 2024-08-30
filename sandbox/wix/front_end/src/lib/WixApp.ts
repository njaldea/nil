import type { Service } from "./Service";

import { nil_wix_proto } from "./proto";

type Range = {
    id: number;
    label: string;
    value: number;
    min: number;
    max: number;
    step: number;
};

type Text = {
    id: number;
    label: string;
    value: string;
    placeholder: string;
};

type Block = {
    label: string;
    widgets: Widget[];
};

type Widget = Block | Text | Range;

type Wix = {
    blocks: Block[];
};

export class WixApp
{
    constructor(service: Service)
    {
        this.service = service;
        this.uplink_handlers = new Map<number, (value: unknown) => void>();
    }

    handle_wix(data: Wix)
    {
        data.blocks.forEach((block) => {});
    }

    // changes from backend
    uplink(id: number, value: unknown)
    {
        const setter = this.uplink_handlers.get(id);
        if (setter != null)
        {
            setter(value);
        }
    }

    // changes from svelte components
    downlink_number(id: number, value: number)
    {
        const header = new Uint8Array(4);
        const data_view = new DataView(header.buffer);
        data_view.setUint32(0, nil_wix_proto.MessageType.MessageType_I64Update, false);
        const message = nil_wix_proto.I64Update.encode({ id, value }).finish();
        const full_buffer = new Uint8Array(header.length + message.length);
        full_buffer.set(header, 0);
        full_buffer.set(message, header.length);
        this.service.publish(full_buffer);
    }
    
    // changes from svelte components
    downlink_string(id: number, value: string)
    {
        const header = new Uint8Array(4);
        const data_view = new DataView(header.buffer);
        data_view.setUint32(0, nil_wix_proto.MessageType.MessageType_StringUpdate, false);
        const message = nil_wix_proto.StringUpdate.encode({ id, value }).finish();
        const full_buffer = new Uint8Array(header.length + message.length);
        full_buffer.set(header, 0);
        full_buffer.set(message, header.length);
        this.service.publish(full_buffer);
    }

    add_uplink_handler(id: number, setter: (value: unknown) => void)
    {
        this.uplink_handlers.set(id, setter);
    }

    remove_uplink_handler(id: number)
    {
        this.uplink_handlers.delete(id);
    }

    uplink_handlers: Map<number, (value: unknown) => void>;
    service: Service;
};

