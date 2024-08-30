import type { Service } from "./Service";

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
        console.log(`downlink n: ${id} ${value}`);
        // this.service.publish(new Uint8Array());
    }
    
    // changes from svelte components
    downlink_string(id: number, value: string)
    {
        console.log(`downlink s: ${id} ${value}`);
        this.service.publish(new Uint8Array());
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

