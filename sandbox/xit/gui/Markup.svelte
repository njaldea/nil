<script>
    // will cause issue here
    // import { writable } from "svelte/store";
    import Div from "./Div.svelte";
    import Range from "./components/Range.svelte";
    import Text from "./components/Text.svelte";
    
    import { binding } from "@nil-/xit";

    const int_binding = binding('binding_0_0', "1101");
    const str_binding = binding('binding_0_1', "world");
</script>

<Div></Div>
<Text bind:value={$str_binding} placeholder="placeholder" label={"text label here"} ></Text>
<Range min={0} max={10} step={1} bind:value={$int_binding} label="label here"></Range>