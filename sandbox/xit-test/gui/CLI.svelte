<script>
    import { xit } from "@nil-/xit";

    const { signals } = xit();
    const send = signals.string("message");
    let command = "";

    const send_signal = () => send(command);
</script>

<div>
    <input
        bind:value={command}
        onkeypress={e => {
            if (e.key === "Enter")
            {
                e.preventDefault();
                send_signal();
            }
        }
    }/>
    <button onclick={send_signal}>SEND</button>
</div>

<style>
    div
    {
        display: flex;
        flex-direction: column;
    }
</style>