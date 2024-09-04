/// <reference lib="webworker" />

self.window = self; // hack for magic-sring and rollup inline sourcemaps

import { Service, header, concat } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

import { resolve_id } from "./resolve_id";
import { load } from "./load";

import { compile } from "svelte/compiler";
import commonjs from './plugins/commonjs.js';
import { rollup, type Plugin } from '@rollup/browser';

import { state } from './state';

const nil_wix_plugin: Plugin = {
    name: 'nil_wix_plugin',
    resolveId: async (importee, importer) => {
        const resolution = await resolve_id(importee, importer);
        // console.log({importee, importer, resolution});
        return resolution;
    },
    load: load,
    transform: async (code, id) => {
        // console.log({transform: id});
        if (id.endsWith(".svelte")) 
        {
            return compile(code, {
                generate: "dom",
                dev: false,
                filename: id,
            }).js;
        }
        return code;
    }
}

self.addEventListener(
    'message',
    async (e) => {
        if (e.data.type == 'init')
        {
            state.service = new Service({
                host: e.data.host,
                port: e.data.port,
                buffer: 1024
            });
            state.service.on_connect(() => {
                console.log('componentize connect');
                state.service!.publish(header(nil_wix_proto.MessageType.MessageType_MarkupRequest));
            });
            state.service.on_disconnect(() => console.log('componentize connect'));
            state.service.on_message((id, data) => {
                const tag = (new DataView(data.buffer)).getUint32(0, false);
                const buffer = data.slice(4);
                if (tag === nil_wix_proto.MessageType.MessageType_MarkupResponse)
                {
                    const response = nil_wix_proto.MarkupResponse.decode(buffer);
                    state.files = response.components;
    
                    rollup({
                        input: '<nil_wix_internal>/index.js',
                        plugins: [
                            nil_wix_plugin,
                            commonjs
                        ],
                        onwarn: (warning) => {} //console.log('warning', warning)
                    })
                    .then(v => v.generate({
                        format: 'es',
                        name: "nil_wix",
                        exports: 'named',
                        sourcemap: 'hidden'
                    }))
                    .then(v => {
                        self.postMessage({
                            ok: true,
                            code: v.output[0].code
                        });
                    })
                    .catch(e => console.log(e));
                }
                else if (tag === nil_wix_proto.MessageType.MessageType_FileResponse)
                {
                    const response = nil_wix_proto.FileResponse.decode(buffer);
                    const result = state.file_content.get(response.target);
                    result!.content = response.content; // for caching
                    result!.resolve!(null);
                }
            });
            state.service.start();
        }
    }
);