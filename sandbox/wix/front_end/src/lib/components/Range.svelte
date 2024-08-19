<script lang="ts">
    let { min, max, value, step, label } = $props<{ min: number, max: number, value: number, step: number, label: string }>();
</script>

<span>{label}</span>
<input type="range" {min} {max} {value} {step}/>