<script>
    let { min, max, value, step, label } = $props();
    let internal_value = $state(value);
</script>

<span>{label}</span>
<input type="range" {min} {max} {step} bind:value={internal_value}/>