import { proto_data } from "./proto";
import { default as Wix } from "./components/Wix.svelte"
import { Service } from "./Service";
import { mount, unmount } from "svelte";

// const messages = globalThis.protobuf.Root.fromJSON(proto_data).lookup("nil.wix.proto");
const messages = (await globalThis.protobuf.load("./wix/sample.proto")).lookup("nil.wix.proto");

export const create = (
    target: Element,
    {port}: {port: number}
) => {
    const component = mount(Wix, { target });
    const service = new Service({ host: 'localhost', port, buffer: 1024 });
    service.on_connect(id => console.log(`connected ${id}`));
    service.on_disconnect(id => console.log(`disconnected ${id}`));

    const handle = (data: Uint8Array) => {
        console.log(data);
        const tag = (new DataView(data.buffer)).getUint32(0, false);
        const buffer = data.slice(4);
        console.log(tag, buffer, messages.MessageType);
        if (tag === messages.MessageType.MessageType_Wix)
        {
            console.log("wtf")
            const { blocks } = messages.Wix.decode(buffer);
            console.log('fff', blocks)
            component.registry.on_block(blocks)
        }
        else
        {
            console.log("unknown type");
        }
    }

    service.on_message((id, data) => handle(data));
    service.start();
    return {
        destroy: () => {
            unmount(component);
            service.stop();
        }
    }
}