import { resolve_id } from "./resolve_id";
import { transform } from "./transform";
import { load } from "./load";

import type { Plugin } from "@rollup/browser";
import type { Options } from "$lib/Service";

const debug_verbose = false;

export const plugin = async (options: Options, files: string[]) => {
    return {
        name: 'nil_wix_plugin',
        resolveId: !debug_verbose
            ? resolve_id
            : async (importee, importer) => {
                console.log('resolve_id', {importee, importer});
                return await resolve_id(importee, importer);
            },
        load: !debug_verbose
            ? await load(options, files)
            : await (async () => {
                const t = await load(options, files);
                return async (resolved) => {
                    console.log('load', {resolved});
                    const resolution = await t(resolved);
                    return resolution;
                }
            })(),
        transform: !debug_verbose
            ? transform()
            : (() => {
                const t = transform()
                return async (code, id) => {
                    console.log('transform', {id});
                    const resolution = await t(code, id);
                    return resolution;
                }
            })()
    } satisfies Plugin;
};