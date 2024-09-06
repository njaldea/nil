<script>
    let { min, max, value, step, label } = $props();
</script>

<span>{label}</span>
<input type="range" {min} {max} {step} bind:value={value}/>