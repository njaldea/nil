<script lang="ts">
    let { label, value, placeholder } = $props<{ label: string; value: string; placeholder: string }>();
</script>

<span>{label}</span>
<input type="text" {value} {placeholder}/>