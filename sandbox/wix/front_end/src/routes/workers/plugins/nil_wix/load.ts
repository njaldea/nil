import { concat, header, service_fetch, type Options } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

const action_script = `
    import { mount, unmount } from 'svelte';
    export const action = (components, contexts) => {
        return (div, props) => {
            let cleanup = components.map((v, i) => mount(v, {target: div, context: contexts[i]}));
            return {
                destroy: () => {
                    cleanup.forEach(v => unmount(v));
                }
            }
        };
    };
`;

const context_script = `
    import { getContext } from "svelte";
    import { writable } from 'svelte/store';
    export const binding = (tag, default_value) => {
        return getContext(tag) ?? default_value;
    };
`;

export const load = async (options: Options, entry: string) => {
    const cache = new Set();
    return async (resolved: string) => {
        if (!cache.has(resolved)) {
            cache.add(resolved);
        } else {
            return null;
        }

        if (resolved === "nil_wix") {
            return context_script;
        }

        if (resolved === "<nil_wix_internal>/index.js") {
            return entry;
        }

        if (resolved === "<nil_wix_internal>/action.js") {
            return action_script;
        }

        if (resolved.startsWith("<nil_wix_user>")) {
            const target = resolved.substring("<nil_wix_user>".length);
            const payload = concat([
                header(nil_wix_proto.MessageType.MessageType_FileRequest),
                nil_wix_proto.FileRequest.encode({ target }).finish()
            ]);
            const response = await service_fetch(options, payload, (tag, data) => {
                if (tag === nil_wix_proto.MessageType.MessageType_FileResponse) {
                    return nil_wix_proto.FileResponse.decode(data);
                }
                throw "err";
            });
            return response.content;
        }

        if (resolved.startsWith("https://unpkg.com")) {
            const content = await (await fetch(resolved)).text();
            return content;
        }

        throw `unknown file: ${resolved}`;
    };
};
