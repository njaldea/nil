import Worker from "./workers/bundler.ts?worker";

type Action = (div: HTMLDivElement) => { destroy: () => void; }; 

export type BundledModule = {
    action: (components: unknown[], contexts: unknown[]) => Action;
    components: unknown[];
};

export const bundle = async (args: { host: string; port: number }) => {
    return new Promise<BundledModule>((resolve, reject) => {
        const worker = new Worker();
        worker.postMessage({ type: "init", host: args.host, port: args.port });
        worker.addEventListener("message", async (e) => {
            if (e.data.ok) {
                resolve(
                    await import(
                        /* @vite-ignore */
                        "data:text/javascript;base64," +
                            btoa(unescape(encodeURIComponent(e.data.code)))
                    )
                );
            } else {
                reject(e.data.err);
            }
            worker.terminate();
        });
    });
};
