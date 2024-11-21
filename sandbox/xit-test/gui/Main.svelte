<svelte:options runes/>

<script>
    import { xit, json_string } from "@nil-/xit";

    const { values, loader } = xit();
    /** @type import("@nil-/xit").Writable<string[]> */
    const tags = values.json("tags", [], json_string);
    /** @type import("@nil-/xit").Writable<string[]> */
    const inputs = values.json("inputs", [], json_string);
    /** @type import("@nil-/xit").Writable<string[]> */
    const outputs = values.json("outputs", [], json_string);
    let selected = $state(-1);

    let sorted_tags = $state($tags.sort());
    tags.subscribe(v => sorted_tags = v.sort());
</script>

<svelte:head>
    <title>nil - xit</title>
</svelte:head>

<div class="root">
    <select bind:value={selected}>
        {#each sorted_tags as id, i}
            <option value={i}>{id}</option>
        {/each}
    </select>

    {#key selected}
        {#if 0 <= selected && selected < $tags.length}
            {@const tag = $tags[selected]}
            {@const i_actions = Promise.all($inputs.map(v => loader.one(v, tag)))}
            {@const o_actions = Promise.all($outputs.map(v => loader.one(v, tag)))}
            {#await Promise.all([ i_actions, o_actions ])}
                <div>Loading...</div>
            {:then [ input_actions, output_actions ]}
                <div class="root-content">
                    <div class="outputs">
                        {#await output_actions then a}
                            {#each a as action}
                                <div style="display: contents" use:action></div>
                            {/each}
                        {/await}
                    </div>
                    <div class="inputs">
                        {#await input_actions then a}
                            {#each a as action}
                                <div style="display: contents" use:action></div>
                            {/each}
                        {/await}
                    </div>
                </div>
            {:catch}
                <div>Error during loading...</div>
            {/await}
        {:else}
            <div>Nothing to load...</div>
        {/if}
    {/key}
</div>

<style>
    .root {
        height: 100%;
        display: flex;
        flex-direction: column;
    }

    .root-content {
        position: relative;
        height: 100%;
    }

    .outputs {
        width: calc(100% - 350px);
    }

    .inputs {
        position: absolute;
        left: calc(100% - 350px);
        right: 0;
        top: 0;
        bottom: 0;
        display: flex;
        flex-direction: column;
    }
</style>