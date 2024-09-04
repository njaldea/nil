<script>
    import Div from "./Div.svelte";
    import Range from "nil/wix/Range.svelte";
    let value = 4;
</script>

<Div></Div>
<Range min={0} max={10} step={1} bind:value={value} label={"label here"}></Range>

