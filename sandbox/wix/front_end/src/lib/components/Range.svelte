<script lang="ts">
    import type { WixApp } from "../WixApp";
    let { app, id, min, max, value, step, label } = $props<{
        app: WixApp;
        id: number;
        min: number;
        max: number;
        value: number;
        step: number;
        label: string;
    }>();

    let internal_value = $state(value);
    app.add_uplink_handler(id, (v: number) => (internal_value = v));
    $effect(() => app.downlink_number(id, internal_value));
</script>

<span>{label}</span>
<input
    type="range"
    {min}
    {max}
    {step}
    bind:value={internal_value}
/>