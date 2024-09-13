<script>
    // can import local files
    import Range from "./components/Range.svelte";
    import Text from "./components/Text.svelte";

    import { getContext } from "svelte";

    /** @type import('./nil-xit').Xit */
    const { binding } = getContext("nil.xit");

    // (binding id, default value) (in case binding is not registered)
    const int_binding = binding.number('binding_0_0', 1101);
    const str_binding = binding.string('binding_0_1', "world");
</script>

<Text bind:value={$str_binding} placeholder="placeholder" label={"text label here"} ></Text>
<Range bind:value={$int_binding} min={0} max={10} step={1} label="label here"></Range>