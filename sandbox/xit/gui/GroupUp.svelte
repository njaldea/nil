<script>
    import { getContext } from "svelte";

    /** @type import('./nil-xit').Xit */
    const { loader } = getContext("nil.xit");

    const frames = ["id-1", "json_editor"];
</script>

<div class="wrapper">
    <!-- {#if frames.length > 0}
        {@const f = loader.all(frames)}
        <div use:f></div>
    {/if} -->
    {#each frames as frame}
        {@const f = loader.one(frame)}
        <div use:f/>
    {/each}
</div>

<style>
    .wrapper {
        display: flex;
        flex-direction: column;
        height: 100%;
        width: 100%;
    }
</style>