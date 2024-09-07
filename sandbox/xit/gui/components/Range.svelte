<script>
    let { min, max, value, step, label } = $props();
</script>

<div>
    <span>{label}</span>
    <input type="range" {min} {max} {step} bind:value={value}/>
</div>