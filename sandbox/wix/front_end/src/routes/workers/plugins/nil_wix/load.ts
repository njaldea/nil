import { concat, header, service_fetch, type Options } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

const action_script = `
    import { mount, unmount } from 'svelte/src/internal/client/render.js';
    export const action = (components) => {
        return (div, props) => {
            let cleanup = components.map(v => mount(v, {target: div}));
            return {
                destroy: () => {
                    cleanup.forEach(v => unmount(v));
                }
            }
        };
    };
`;

const populate_wix_root = (files: string[]) => {
    const import_lines = files
        .map((v, i) => `import Component_${i} from "<nil_wix_user>${v}";`)
        .join('\n');
    const result = [
        import_lines,
        `export { action } from "<nil_wix_internal>/action.js";`,
        `export const components = [${files.map((v, i) => `Component_${i}`).join(', ')}];`
    ].join('\n');
    return result;
};

export const load = async (options: Options, files: string[]) => {
    const cached_user_files = new Map<string, string>();
    for (const target of files)
    {
        const payload = concat([
            header(nil_wix_proto.MessageType.MessageType_FileRequest),
            nil_wix_proto.FileRequest.encode({ target }).finish()
        ])
        cached_user_files.set(`<nil_wix_user>${target}`, await service_fetch(options, payload, (data) => {
            const tag = (new DataView(data.buffer)).getUint32(0, false);
            const buffer = data.slice(4);
            if (tag === nil_wix_proto.MessageType.MessageType_FileResponse)
            {
                return nil_wix_proto.FileResponse.decode(buffer).content;
            }
            throw "err";
        }));
    }
    const cache = new Set();
    return async (resolved: string) => {
        if (!cache.has(resolved)) {
            cache.add(resolved);
        } else {
            return null;
        }

        if (resolved === "<nil_wix_internal>/index.js") {
            return populate_wix_root(files);
        }

        if (resolved === "<nil_wix_internal>/action.js") {
            return action_script;
        }

        if (cached_user_files.has(resolved))
        {
            return cached_user_files.get(resolved);
        }
    
        if (resolved.startsWith("<nil_wix_user>")) {
            const target = resolved.substring("<nil_wix_user>".length);
            const payload = concat([
                header(nil_wix_proto.MessageType.MessageType_FileRequest),
                nil_wix_proto.FileRequest.encode({ target }).finish()
            ])
            const response = await service_fetch(options, payload, (data) => {
                const tag = (new DataView(data.buffer)).getUint32(0, false);
                const buffer = data.slice(4);
                if (tag === nil_wix_proto.MessageType.MessageType_FileResponse)
                {
                    return nil_wix_proto.FileResponse.decode(buffer);
                }
                throw "err";
            });
            return response.content;
        }
    
        if (resolved.startsWith('https://unpkg.com'))
        {
            const content = await (await fetch(resolved)).text();
            return content;
        }
    
        throw `unknown file: ${resolved}`;
    }
};
