<script>
    let { label, placeholder, value } = $props();
</script>

<div>
    <span>{label}</span>
    <input bind:value={value} placeholder={placeholder}/>
</div>