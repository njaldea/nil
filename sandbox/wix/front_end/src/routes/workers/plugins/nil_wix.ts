import { resolve_id } from "../resolve_id";
import { load } from "../load";

import { compile } from "svelte/compiler";
import type { Plugin } from "@rollup/browser";

const debug_verbose = false;

export const plugin: Plugin = {
    name: 'nil_wix_plugin',
    resolveId: !debug_verbose
        ? resolve_id
        : async (importee, importer) => {
            const resolution = await resolve_id(importee, importer);
            console.log({importee, importer, resolution});
            return resolution;
        },
    load: load,
    transform: async (code, id) => {
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