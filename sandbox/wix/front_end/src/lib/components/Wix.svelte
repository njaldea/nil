<script lang="ts">
    import Block from "./Block.svelte";
    import type { Widget } from "./types";
    
    type BlockType = { label: string; widgets: Widget[] };
    let { blocks } = $props<{ blocks: BlockType[] }>();
</script>

{#each blocks as block}
    <Block label={block.label} widgets={block.widgets}></Block>
{/each}