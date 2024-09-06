<script lang="ts">
    import Client from "./Client.svelte";
    import { browser } from "$app/environment";
</script>

{#if browser}
    <Client host={"localhost"} port={1101} ></Client>
{/if}
