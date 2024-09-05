<script lang="ts">
    import { bundle } from "./bundle";

    let { port, host, route } = $props<{
        port: number;
        host: string;
        route?: string;
    }>();

    const call = (e: Error) => {
        console.log(e.stack);
        return "Something went wrong..."
    }
</script>

{#await bundle({ host, port })}
    Loading...
{:then { mount_me }} 
    <div use:mount_me></div>
{:catch e}
    {call(e)}
{/await}
