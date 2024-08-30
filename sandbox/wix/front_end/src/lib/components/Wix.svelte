<script lang="ts">
    import Block from "./Block.svelte";
    import type { Widget } from "./types";
    import type { WixApp } from "../WixApp";
    
    type BlockType = { label: string; widgets: Widget[] };
    let { app, blocks } = $props<{
        app: WixApp;
        blocks: BlockType[];
    }>();
</script>

<div style="display:flex; flex-direction: column; max-width: 400px;">
    {#each blocks as block}
        <Block {...block} {app}></Block>
    {/each}
</div>