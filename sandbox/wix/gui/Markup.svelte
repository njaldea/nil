<script>
    import Div from "./Div.svelte";
    import Range from "./components/Range.svelte";
    
    import { jwalker } from "@nil-/jwalk";

    const j = jwalker()
        .node("Boolean", "boolean", {
            action: ({ value }) => {
                console.log("[BOOL]", "INIT", value);
                return {
                    update: (value) => console.log("[BOOL]", "UPDATE", value),
                    destroy: () => console.log("[BOOL]", "DESTROY", "-")
                };
            }
        })
        .node("Number", "number", {
            action: ({ value }) => {
                console.log("[Number]", "INIT", value);
                return {
                    update: (value) => console.log("[Number]", "UPDATE", value),
                    destroy: () => console.log("[Number]", "DESTROY", "-")
                };
            }
        })
        .node("ROOT", "tuple", {
            content: ["Boolean", "Number"],
            action: ({ value, auto }) => {
                console.log("[ROOT]", "INIT", value);
                const { update, destroy } = auto(
                    () => {},
                    () => {},
                    value
                );
                return {
                    update: (value) => {
                        console.log("[ROOT]", "UPDATE", value);
                        update(value);
                    },
                    destroy: () => {
                        destroy();
                        console.log("[ROOT]", "DESTROY", "-");
                    }
                };
            }
        })
        .build(null, [true, 100]);

    j.update([false, 200]);

    j.destroy();

    let value = 4;
</script>

<Div></Div>
<Range min={0} max={10} step={1} bind:value={value} label={"label here"}></Range>