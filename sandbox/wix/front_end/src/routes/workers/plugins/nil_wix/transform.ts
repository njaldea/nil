import { compile } from "svelte/compiler";

export const transform = () => {
    const cache = new Set<string>();
    return (code: string, id: string) => {
        if (cache.has(id)) {
            return null;
        }
        cache.add(id);
        if (id.endsWith(".svelte")) 
        {
            const result = compile(code, {
                generate: "dom",
                dev: false,
                filename: id,
            });
            return { code: result.js.code, map: null };
        }
        return { code, map: null };
    };
};