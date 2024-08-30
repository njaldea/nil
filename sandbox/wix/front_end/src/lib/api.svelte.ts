import { default as Wix } from "./components/Wix.svelte";
import { WixApp } from "./WixApp";
import { Service } from "./Service";
import { mount, unmount } from "svelte";

import { nil_wix_proto } from "./proto";

export const create = (target: Element) => {
    const service = new Service({ route: '/ws', host: location.host, buffer: 1024 });
    const app = new WixApp(service);

    let component: any = null;
    let props = $state({ blocks: [], app });

    service.on_connect(() => (component = mount(Wix, { target, props })));
    service.on_disconnect(() => (component != null) && unmount(component));
    service.on_message((id, data) => {
        const tag = (new DataView(data.buffer)).getUint32(0, false);
        const buffer = data.slice(4);
        if (tag === nil_wix_proto.MessageType.MessageType_Wix)
        {
            props.blocks = nil_wix_proto.Wix.decode(buffer).blocks;
        }
    });
    service.start();

    return { destroy: () => service.stop() };
}