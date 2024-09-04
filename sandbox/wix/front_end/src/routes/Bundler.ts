import Worker from './workers/bundler.ts?worker';

export const bundle = (args: {
    host: string;
    port: number;
}) => {
    const worker = new Worker();
    worker.postMessage({ type: 'init', host: args.host, port: args.port });
    let resolve: null | ((arg: any) => void) = null;
    let reject: null | (() => void) = null;
    worker.addEventListener('message', (e) => {
        if (e.data.ok)
        {
            if (resolve != null)
            {
                const module_data = e.data.code;
                const b64_module_data = "data:text/javascript;base64," + btoa(unescape(encodeURIComponent(module_data)));
                resolve(import(b64_module_data /* @vite-ignore */));
            }
        }
        else
        {
            if (reject != null)
            {
                reject();
            }
        }
    });
    return {
        cancel: () => {
            worker.terminate();
            if (reject != null)
            {
                reject();
            }
        },
        m: new Promise<{ mount_me: () => void }>((r, x) => {
            resolve = r;
            reject = x;
        })
    }
}
