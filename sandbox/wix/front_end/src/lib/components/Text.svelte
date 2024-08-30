<script lang="ts">
    import type { WixApp } from "../WixApp";
    let { app, id, label, value, placeholder } = $props<{
        app: WixApp;
        id: Number;
        label: string;
        value: string;
        placeholder: string;
    }>();

    let internal_value = $state(value);
    app.add_uplink_handler(id, (v: string) => (internal_value = v));
    let first = true;
    $effect(() => {
        internal_value;
        if (!first)
        {
            app.downlink_string(id, internal_value);
        }
        first = false;
    });
</script>

<span>{label}</span>
<input
    type="text"
    bind:value={internal_value}
    {placeholder}
/>