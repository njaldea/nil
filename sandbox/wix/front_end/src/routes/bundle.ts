import type { Action } from 'svelte/action';
import Worker from './workers/bundler.ts?worker';

import { writable } from 'svelte/store';

export const bundle = async (args: {
    host: string;
    port: number;
}) => {
    return new Promise<{ mount_me: Action<HTMLDivElement> }>((resolve, reject) => {
        const worker = new Worker();
        worker.postMessage({ type: 'init', host: args.host, port: args.port });
        worker.addEventListener('message', async (e) => {
            if (e.data.ok)
            {
                const m = await import(
                    /* @vite-ignore */
                    "data:text/javascript;base64," + btoa(unescape(encodeURIComponent(e.data.code)))
                );
                const context = new Map();
                context.set("binding_tag", writable(300));
                resolve({ mount_me: m.action(m.components, [context]) });
            }
            else
            {
                reject();
            }
        });
    });
}
