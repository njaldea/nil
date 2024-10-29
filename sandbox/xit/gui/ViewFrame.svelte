<script>
    import "https://cdn.plot.ly/plotly-2.35.2.min.js";
    import { xit, json_string } from "@nil-/xit";

    const { values } = xit();

    const scene = values.json("scene", {}, json_string);

    const plot_it = (target, props) => {
        window.Plotly.newPlot(target, props);
        return {
            update: (new_props) => window.Plotly.react(target, new_props),
            destroy: () => window.Plotly.purge(target)
        }
    };

</script>

<svelte:head>
    <title>nil - xit - view</title>
</svelte:head>

<div use:plot_it={$scene}></div>