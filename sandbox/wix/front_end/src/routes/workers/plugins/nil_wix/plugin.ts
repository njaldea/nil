import { resolve_id } from "./resolve_id";
import { transform } from "./transform";
import { load } from "./load";

import type { Plugin } from "@rollup/browser";
import type { Options } from "$lib/Service";

const debug_verbose = false;

export const plugin = async (options: Options, files: string[], entry: string) => {
    return {
        name: 'nil_wix_plugin',
        resolveId: !debug_verbose
            ? await resolve_id()
            : await (async () => {
                const t = await resolve_id();
                return async (importee, importer) => {
                    const resolution = await t(importee, importer);
                    if (resolution != null) {
                        console.log('resolve_id', {importee, importer, resolution});
                    } else {
                        console.log('resolve issue', { importee, importer });
                    }
                    return resolution;
                };
            })(),
        load: !debug_verbose
            ? await load(options, files, entry)
            : await (async () => {
                const t = await load(options, files, entry);
                return async (resolved) => {
                    const resolution = await t(resolved);
                    console.log('load', {resolved, resolution});
                    return resolution;
                }
            })(),
        transform: !debug_verbose
            ? transform()
            : (() => {
                const t = transform()
                return async (code, id) => {
                    const resolution = await t(code, id);
                    console.log('transform', {id, resolution});
                    return resolution;
                }
            })()
    } satisfies Plugin;
};