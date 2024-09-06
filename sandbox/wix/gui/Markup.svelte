<script>
    // will cause issue here
    // import { writable } from "svelte/store";
    import Div from "./Div.svelte";
    import Range from "./components/Range.svelte";
    
    import { binding } from "nil_wix";

    const ff = binding('binding_tag', 100);

    let value = 4;
</script>

<Div></Div>
<Range min={0} max={10} step={1} bind:value={value} label={"label here"}></Range>