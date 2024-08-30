<script lang="ts">
    import Range from "./Range.svelte";
    import Text from "./Text.svelte";
    import type { Widget } from "./types";
    import { WixApp } from "../WixApp";

    let { app, label, widgets } = $props<{ app: WixApp; label: string; widgets: Widget[] }>();
</script>

<span>{label}</span>
<div style="display:flex; flex-direction: column;">
    {#each widgets as widget}
        {#if null != widget.block}
            <svelte:self {...widget.block} {app}/>
        {:else if null != widget.range}
            <Range {...widget.range} {app}></Range>
        {:else if null != widget.text}
            <Text {...widget.text} {app}></Text>
        {/if}
    {/each}
</div>
