<script>
    import { xit } from "@nil-/xit";

    const { signals } = xit();
    const send = signals.string("message");
    let command = "";
</script>

<div>
    <input bind:value={command}/>
    <button onclick={() => send(command)}>SEND</button>
</div>

<style>
    div
    {
        display: flex;
        flex-direction: column;
    }
</style>