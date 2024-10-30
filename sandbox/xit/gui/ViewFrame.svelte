<script>
    import { xit, json_string } from "@nil-/xit";
    import "https://cdn.plot.ly/plotly-2.35.2.min.js";

    const { values } = xit();

    const scene = values.json("scene", {}, json_string);

    const plot_it = (target, props) => {
        window.Plotly.newPlot(target, props);
        const observer = new ResizeObserver(() => window.Plotly.Plots.resize(target));
        observer.observe(target);
        return {
            update: (new_props) => window.Plotly.react(target, new_props),
            destroy: () => {
                observer.unobserve(target);
                window.Plotly.purge(target);
            }
        };
    };
</script>

<svelte:head>
    <title>nil - xit - view</title>
</svelte:head>

<div use:plot_it={$scene}></div>