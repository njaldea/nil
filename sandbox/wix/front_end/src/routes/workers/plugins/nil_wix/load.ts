import { concat, header, service_fetch, type Options } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

const mount_me_script = `
    import { mount, unmount } from 'svelte';
    export const mount_me_maker = (div, props, components) => {
        let cleanup = components.map(v => mount(v, {target: div}));
        return {
            destroy: () => {
                cleanup.forEach(v => unmount(v));
            }
        }
    };
`;

const populate_wix_root = (files: string[]) => {
    const import_lines = files
        .map((v, i) => `import Component_${i} from "<nil_wix_user>${v}";`)
        .join('\n');
    const result = [
        import_lines,
        `import { mount, unmount } from 'svelte';`,
        // `import { mount_me_maker } from "<nil_wix_internal>/mount_me_maker.js";`,
        `const nil_wix = [${files.map((v, i) => `Component_${i}`).join(', ')}];`,
        `export const mount_me = (div, props) => {
            let cleanup = nil_wix.map(v => mount(v, {target: div}));
            return {
                destroy: () => {
                    cleanup.forEach(v => unmount(v));
                }
            }
        };`
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
        if (resolved === "<nil_wix_internal>/index.js") {
            cache.add(resolved);
            return populate_wix_root(files);
        }

        if (cache.has(resolved))
        {
            return null;
        }

        if (cached_user_files.has(resolved))
        {
            cache.add(resolved);
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
            cache.add(resolved);
            return response.content;
        }
    
        if (resolved.startsWith('https://unpkg.com'))
        {
            const content = await (await fetch(resolved)).text();
            cache.add(resolved);
            return content;
        }
    
        throw `unknown file: ${resolved}`;
    }
};
