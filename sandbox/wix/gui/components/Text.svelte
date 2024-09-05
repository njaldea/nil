<script>
    let { label, placeholder } = $props();
    let value = "";
</script>

<span>{label}</span>
<input bind={value} placeholder={placeholder}/>