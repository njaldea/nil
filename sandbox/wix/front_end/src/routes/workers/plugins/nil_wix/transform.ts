import { compile } from "svelte/compiler";

export const transform = () => {
    const cache = new Set<string>();
    return (code: string, id: string) => {
        if (!cache.has(id)) {
            cache.add(id);
        } else {
            return null;
        }
        if (id.endsWith(".svelte")) 
        {
            const result = compile(code, {
                generate: "client",
                dev: false,
                filename: id
            });
            return { code: result.js.code, map: null };
        }
        return null;
    };
};