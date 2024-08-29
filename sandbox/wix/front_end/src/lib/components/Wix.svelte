<script lang="ts">
    import Block from "./Block.svelte";
    import type { Widget } from "./types";
    
    type BlockType = { label: string; widgets: Widget[] };
    let { blocks } = $props<{ blocks: BlockType[] }>();
</script>

<div style="display:flex; flex-direction: column; max-width: 400px;">
    {#each blocks as block}
        <Block {...block}></Block>
    {/each}
</div>