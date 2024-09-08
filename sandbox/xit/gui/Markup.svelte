<script>
    // can import local files
    import Range from "./components/Range.svelte";
    import Text from "./components/Text.svelte";

    // this is in npmjs specifically published for this project
    // - https://www.npmjs.com/package/@nil-/xit
    import { binding } from "@nil-/xit";

    // can import anything in npmjs (served through unpkg.com)

    // bindingid, default value (in case binding is not registered)
    const int_binding = binding('binding_0_0', "1101");
    const str_binding = binding('binding_0_1', "world");
</script>

<Text bind:value={$str_binding} placeholder="placeholder" label={"text label here"} ></Text>
<Range min={0} max={10} step={1} bind:value={$int_binding} label="label here"></Range>