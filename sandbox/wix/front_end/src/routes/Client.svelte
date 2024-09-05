<script lang="ts">
    import { bundle } from "./bundle";
    import { mount, unmount } from "svelte";

    let { port, host, route } = $props<{
        port: number;
        host: string;
        route?: string;
    }>();

</script>

{#await bundle({ host, port })}
    Loading...
{:then { mount_me }} 
    <div use:mount_me></div>
{:catch}
    Something went wrong...
{/await}
