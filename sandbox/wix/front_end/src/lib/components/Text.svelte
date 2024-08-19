<script lang="ts">
    let { value, placeholder } = $props<{ value: string; placeholder: string }>();
</script>

<input type="text" {value} {placeholder}/>