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
                const module_data = e.data.code;
                const b64_module_data = "data:text/javascript;base64," + btoa(unescape(encodeURIComponent(module_data)));
                resolve(import(b64_module_data /* @vite-ignore */));
            }
            else
            {
                reject();
            }
        });
    });
}
