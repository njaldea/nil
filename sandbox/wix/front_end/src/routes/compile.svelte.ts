import { Service, header, concat } from "$lib/Service";

import { nil_wix_proto } from "$lib/proto";

export const componentize = (
    host: string,
    port: number,
    file_list: (files: string[]) => void,
    file_handler: (tag: string, content: string) => void
) => {
    const service = new Service({ host, port, buffer: 1024 });
    service.on_connect(() => {
        console.log('componentize connect');
        service.publish(header(nil_wix_proto.MessageType.MessageType_MarkupRequest));
    });
    service.on_disconnect(() => console.log('componentize connect'));
    service.on_message((id, data) => {
        const tag = (new DataView(data.buffer)).getUint32(0, false);
        const buffer = data.slice(4);
        if (tag === nil_wix_proto.MessageType.MessageType_MarkupResponse)
        {
            const response = nil_wix_proto.MarkupResponse.decode(buffer);
            file_list(response.components);
            for (const component of response.components)
            {
                console.log(`requesting ${component}`);
                service.publish(concat([
                    header(nil_wix_proto.MessageType.MessageType_FileRequest),
                    nil_wix_proto.FileRequest.encode({ target: component }).finish()
                ]));
            }
        }
        else if (tag === nil_wix_proto.MessageType.MessageType_FileResponse)
        {
            const response = nil_wix_proto.FileResponse.decode(buffer);
            file_handler(response.target, response.content);
        }
    });
    service.start();
    return {
        destroy: () => service.stop()
    }
}