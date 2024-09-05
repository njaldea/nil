import type { Action } from 'svelte/action';
import Worker from './workers/bundler.ts?worker';

export const bundle = async (args: {
    host: string;
    port: number;
}) => {
    return new Promise<{ mount_me: Action<HTMLDivElement> }>((resolve, reject) => {
        const worker = new Worker();
        worker.postMessage({ type: 'init', host: args.host, port: args.port });
        worker.addEventListener('message', (e) => {
            if (e.data.ok)
            {
                resolve(import(
                    /* @vite-ignore */
                    "data:text/javascript;base64," + btoa(unescape(encodeURIComponent(e.data.code)))
                ));
            }
            else
            {
                reject();
            }
        });
    });
}
