/// <reference lib="webworker" />

self.window = self; // hack for magic-sring and rollup inline sourcemaps

import { Service, header } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

import { plugin as commonjs } from './plugins/commonjs';
import { plugin as nil_wix } from './plugins/nil_wix';
import { rollup } from '@rollup/browser';

import { state } from './state';

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
                            nil_wix,
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