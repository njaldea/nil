<script>
    import { xit, json_string } from "@nil-/xit";
    import "https://cdn.plot.ly/plotly-2.35.2.min.js";

    const { values } = xit();

    const value_x = values.json("value-x", null, json_string);
    const value_y = values.json("value-y", null, json_string);

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

{#if $value_x != null && $value_y != null}
    <div use:plot_it={[{ x: $value_x, y:$value_y, type: "bar"}]}></div>
{/if}

<style>
    div {
        height: 100%;
    }
</style>