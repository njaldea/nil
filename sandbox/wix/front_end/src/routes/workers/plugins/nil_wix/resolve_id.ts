import { exports } from 'resolve.exports';

type PACKAGE_INFO = {
    pkg: string;
    base: string;
    link: string;
    rest: string;
    json: any;
};
const JSON_PKG_CACHE = new Map<string, PACKAGE_INFO>();

const fetch_package_json = async (target: string) => {
    const unpkg = `https://unpkg.com`;
    const split_importee = target.split('/');
    const ff = async (i: number) => {
        try
        {
            const pkg = split_importee.slice(0, i + 1).join('/');
            const base = `${unpkg}/${pkg}`;
            const link = `${base}/package.json`;
            if (JSON_PKG_CACHE.has(link)) {
                return JSON_PKG_CACHE.get(link);
            }
            const result = await fetch(link);
            if (result.ok) {
                JSON_PKG_CACHE.set(link, {
                    pkg,
                    base,
                    link,
                    rest: split_importee.slice(i + 1).join('/'),
                    json: JSON.parse(await result.text())
                });
                return JSON_PKG_CACHE.get(link);
            }
        }
        catch (e)
        {
            console.log(e);
        }
        if (i < target.length) {
            return ff(i + 1);
        }
        throw "can't find";
    };
    return ff(0);
};

export const resolve_id = async (importee: string, importer?: string) => {
    if (importee.startsWith("<nil_wix_internal>")) {
        return importee;
    }

    if (importee.startsWith("<nil_wix_user>"))
    {
        // this will be triggered only for root stuff.
        return importee;
    }

    if (importee.startsWith("nil/wix/")) {
        return `<nil_wix_internal>/components/${importee.slice(8)}`;
    }

    if (importee.startsWith("svelte")) {
        importee = `svelte@5.0.0-next.222` + importee.slice(6);
    }

    if (importee.startsWith('svelte@5.0.0-next.222/src')) {
        return `https://unpkg.com/${importee}`; // bypass
    }

    if (importee.startsWith(".")) {
        if (importer?.startsWith('<nil_wix_user>')) {
            const base = new URL(`${location.origin}/${importer.slice(15)}`);
            const resolved = (new URL(importee, base)).href;
            return `<nil_wix_user>${resolved.slice(location.origin.length)}`;
        }

        if (importer?.startsWith('https://unpkg.com')) {
            return (await fetch(
                new URL(importee, importer),
                { method: "HEAD" })
            ).url;
        }
    }

    const result = await fetch_package_json(importee);
    if (result != null)
    {
        // string composition works because result.base is always `http://unpkg.com/module`
        // url resolution will always remove the last portion of the link which
        // is not applicable in this case
        const path = exports(result.json, result.rest, {browser: true, conditions: ['svelte', 'production']});
        if (Array.isArray(path) && path.length > 0) {
            return (await fetch(new URL(`${result.base}/${path[0]}`), { method: "HEAD" })).url;
        } else if (typeof path === 'string') {
            return (await fetch(new URL(`${result.base}/${path}`), { method: "HEAD" })).url;
        }
    }

    throw "unknown file, should be unreachable";
}