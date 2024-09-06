/// <reference lib="webworker" />

self.window = self; // hack for magic-sring and rollup inline sourcemaps

import { service_fetch, header } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

import { plugin as commonjs } from './plugins/commonjs/plugin';
import { plugin as nil_wix } from './plugins/nil_wix/plugin';
import { rollup } from '@rollup/browser';

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

self.addEventListener(
    'message',
    async (e) => {
        if (e.data.type == 'init')
        {
            const options = {
                host: e.data.host,
                port: e.data.port,
                buffer: 1024
            };

            const files = await service_fetch(
                options,
                header(nil_wix_proto.MessageType.MessageType_MarkupRequest),
                (data) => {
                    const tag = (new DataView(data.buffer)).getUint32(0, false);
                    const buffer = data.slice(4);
                    if (tag === nil_wix_proto.MessageType.MessageType_MarkupResponse)
                    {
                        return nil_wix_proto.MarkupResponse.decode(buffer).components;
                    }
                    throw 'err';
                }
            );

            const r = await rollup({
                input: '<nil_wix_internal>/index.js',
                plugins: [
                    await nil_wix(options, files, populate_wix_root(files)),
                    commonjs(),
                ],
                external: [
                    'svelte',
                    'svelte/store',
                    'svelte/internal/client',
                    'svelte/internal/disclose-version'
                ],
                logLevel: "debug",
                onwarn: (warning) => false && console.log('warning', warning)
            });
            const g = await r.generate({ format: 'esm' });
            const f = await rollup({
                input: '<nil_wix_internal>/index.js',
                plugins: [
                    await nil_wix(options, [], g.output[0].code),
                    commonjs(),
                ],
                logLevel: "debug",
                onwarn: (warning) => false && console.log('warning', warning)
            });
            const gg = await f.generate({ format: 'esm' });
            self.postMessage({
                ok: true,
                code: gg.output[0].code
            });
        }
    }
);