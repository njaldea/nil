import { state, populate_wix_root } from "./state";

import { concat, header } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

export const load = async (resolved: string) => {
    if (resolved == "<nil_wix_internal>/index.js") {
        return populate_wix_root();
    }

    if (state.file_content.has(resolved))
    {
        return state.file_content.get(resolved)!.content;
    }

    if (resolved.startsWith("<nil_wix_user>")) {
        const target = resolved.substring("<nil_wix_user>".length);
        if (state.file_content.has(target))
        {
            return state.file_content.get(target)!.content;
        }
        else
        {
            state.service!.publish(concat([
                header(nil_wix_proto.MessageType.MessageType_FileRequest),
                nil_wix_proto.FileRequest.encode({ target }).finish()
            ]));
            await (new Promise((resolve, reject) => {
                state.file_content.set(target, { resolve, reject, content: null });
            }));
            return state.file_content.get(target)!.content;
        }
    }

    if (resolved.startsWith('https://unpkg.com'))
    {
        const content = await (await fetch(resolved)).text();
        state.file_content.set(resolved, { content: content });
        return content;
    }

    throw `unknown file: ${resolved}`;
}