<script lang="ts">
    import { bundle } from "./Bundler";
    import { mount, unmount } from "svelte";

    let { port, host, route } = $props<{
        port: number;
        host: string;
        route?: string;
    }>();

    let mount_me: null | any = $state(null);

    const do_it = async () => {
        const module_data = await bundle({ host, port }).m;
        mount_me = module_data.mount_me;
    };

    do_it();
</script>

{#if mount_me != null}
  <div use:mount_me></div>
{/if}
