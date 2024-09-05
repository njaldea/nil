import { Service } from "$lib/Service";

type State = {
    service: null | Service;
    files: string[];
    file_content: Map<string, { resolve?: (v: unknown) => void; reject?: () => void; content: null | string }>;
}

export const state: State = {
    service: null,
    files: [],
    file_content: new Map()
};

const mount_me = `
    export const mount_me = (div, props) => {
        let cleanup = nil_wix.map(v => mount(v, {target: div}));
        return {
            destroy: () => {
                cleanup.forEach(v => unmount(v));
            }
        }
    };`;

export const populate_wix_root = () => {
    const result = [
        state.files
            .map((v, i) => `import Component_${i} from "<nil_wix_user>${v}";`)
            .join('\n'),
        `import { mount, unmount } from 'svelte';`,
        `const nil_wix = [${state.files.map((v, i) => `Component_${i}`).join(', ')}];`,
        mount_me
    ].join('\n');
    return result;
};