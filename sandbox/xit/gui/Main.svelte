<script>
    import { xit, json_string } from "@nil-/xit";

    const { values, loader } = xit();

    const scenes = values.json("scenes", { scenes: [] }, json_string);

    let selected = $state(null);
</script>

<svelte:head>
    <title>nil - xit - test</title>
</svelte:head>

<div class="root">
    <select bind:value={selected}>
        {#each $scenes.scenes as id, i}
            <option value={i}>{id}</option>
        {/each}
    </select>

    {#key selected}
        {#if selected != null}
            {@const f = frames[selected]}
            {#await loader.one("test_frame", $scenes.scenes[selected])}
                <div>loading...</div>
            {:then a}
                <div use:a></div>
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