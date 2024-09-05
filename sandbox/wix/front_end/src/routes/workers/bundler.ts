/// <reference lib="webworker" />

self.window = self; // hack for magic-sring and rollup inline sourcemaps

import { Service, service_fetch, header } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

import { plugin as commonjs } from './plugins/commonjs/plugin';
import { plugin as nil_wix } from './plugins/nil_wix/plugin';
import { rollup } from '@rollup/browser';

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

            rollup({
                input: '<nil_wix_internal>/index.js',
                plugins: [
                    await nil_wix(options, files),
                    commonjs
                ],
                onwarn: (warning) => {} // console.log('warning', warning)
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
    }
);