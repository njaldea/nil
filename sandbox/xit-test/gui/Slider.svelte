<script>
    import { xit } from "@nil-/xit";
    const { values } = xit();
    const v_1 = values.number("value-1", 0);
    const v_2 = values.number("value-2", 0);
    const v_3 = values.number("value-3", 0);
</script>

<input type="range" min={0} max={10} step={1} bind:value={$v_1}/>
<input type="range" min={0} max={10} step={1} bind:value={$v_2}/>
<input type="range" min={0} max={10} step={1} bind:value={$v_3}/>