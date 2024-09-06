/// <reference lib="webworker" />

self.window = self; // hack for magic-sring and rollup inline sourcemaps

import { service_fetch, header, type Options } from "$lib/Service";
import { nil_wix_proto } from "$lib/proto";

import { plugin as commonjs } from "./plugins/commonjs/plugin";
import { plugin as nil_wix, warning } from "./plugins/nil_wix/plugin";
import { rollup } from "@rollup/browser";

const populate_wix_root = (files: string[]) => {
    const result = [
        ...files.map((v, i) => `import Component_${i} from "<nil_wix_user>${v}";`),
        `export { action } from "<nil_wix_internal>/action.js";`,
        `export const components = [${files.map((v, i) => `Component_${i}`).join(", ")}];`
    ].join("\n");
    return result;
};

const bundle = async (options: Options, entry: string, external?: (v: string) => boolean) => {
    const r = await rollup({
        input: "<nil_wix_internal>/index.js",
        plugins: [await nil_wix(options, entry), commonjs()],
        external,
        onwarn: warning
    });
    const g = await r.generate({ format: "esm" });
    return g.output[0].code;
};

self.addEventListener("message", async (e) => {
    try {
        if (e.data.type == "init") {
            const options = { host: e.data.host, port: e.data.port };

            const files = await service_fetch(
                options,
                header(nil_wix_proto.MessageType.MessageType_MarkupRequest),
                (tag, data) => {
                    if (tag === nil_wix_proto.MessageType.MessageType_MarkupResponse) {
                        return nil_wix_proto.MarkupResponse.decode(data).components;
                    }
                    throw "err";
                }
            );

            const root_entry = populate_wix_root(files);
            const bypass_svelte_files = (e: string) => e.startsWith("svelte");
            const initial_pass = await bundle(options, root_entry, bypass_svelte_files);
            const final_output = await bundle(options, initial_pass);

            self.postMessage({ ok: true, code: final_output });
        }
    } catch (e) {
        self.postMessage({ ok: false, err: "Error somewhere...." });
    }
});
