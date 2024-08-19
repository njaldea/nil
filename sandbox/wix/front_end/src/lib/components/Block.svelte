<script lang="ts">
    import Range from "./Range.svelte";
    import Text from "./Text.svelte";
    import type { Widget } from "./types";
    let { label, widgets } = $props<{ label: string; widgets: Widget[] }>();
</script>

<span>{label}</span>
<div style="display:flex; flex-direction: column;">
    {#each widgets as widget}
        {#if null != widget.block}
            <svelte:self label={widget.block.label} widgets={widget.block.widgets}/>
        {:else if null != widget.range}
            <Range {...widget.range}></Range>
        {:else if null != widget.text}
            <Text {...widget.text}></Text>
        {/if}
    {/each}
</div>
