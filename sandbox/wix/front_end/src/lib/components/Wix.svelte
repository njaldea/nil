<script lang="ts">
    import Block from "./Block.svelte";
    import type { Widget } from "./types";
    
    type BlockType = { label: string; widgets: Widget[] };
    let blocks = $state<BlockType[]>([]);

    export const registry = {
        on_block: (new_blocks: BlockType[]) => {
            blocks = new_blocks;
        }
    };
</script>

{#each blocks as { label, widgets }}
<Block {label} {widgets}></Block>
{/each}