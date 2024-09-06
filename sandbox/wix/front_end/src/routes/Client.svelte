<script lang="ts">
    import { bundle, type BundledModule } from "./bundle";
    import { writable } from "svelte/store";

    let { port, host, route } = $props<{
        port: number;
        host: string;
        route?: string;
    }>();

    const call = (e: Error) => {
        console.log(e.stack);
        return "Something went wrong...";
    };

    const bundle_step = async () => {
        const m = await bundle({ host, port });
        const context = new Map();
        context.set("binding_tag", writable(300));
        return m.action(m.components, [context]);
    };
</script>

{#await bundle_step()}
    Loading...
{:then action}
    <div use:action></div>
{:catch e}
    {call(e)}
{/await}
