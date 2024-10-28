<script>
    import { xit, json_string } from "@nil-/xit";

    const { values, loader } = xit();
    const scenes = values.json("scenes", { scenes: [""] }, json_string);
    const selected = values.number("selected", 0);
</script>

<svelte:head>
    <title>nil - xit</title>
</svelte:head>

<div class="root">
    <select bind:value={$selected}>
        {#each $scenes.scenes as id, i}
            <option value={i}>{id}</option>
        {/each}
    </select>

    {#key $selected}
        {#if 0 < $selected && $selected < $scenes.scenes.length}
            {#await loader.one("view_frame", $scenes.scenes[$selected])}
                <div>Loading...</div>
            {:then a}
                <div style="display: contents" use:a></div>
            {:catch}
                <div>Error during loading...</div>
            {/await}
        {/if}
    {/key}
</div>

<style>
    .root {
        display: flex;
        flex-direction: column;
    }
</style>