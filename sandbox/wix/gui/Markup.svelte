<script>
    // will cause issue here
    // import { writable } from "svelte/store";
    import Div from "./Div.svelte";
    import Range from "./components/Range.svelte";
    
    import { binding } from "nil_wix";

    const int_binding = binding('binding_0_0', "1101");
    const str_binding = binding('binding_0_1', "world");
</script>

<Div></Div>
<Range min={0} max={10} step={1} bind:value={$int_binding} label={"label here"}></Range>