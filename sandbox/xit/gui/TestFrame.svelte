<script>
    import { xit, json_string } from "@nil-/xit";

    const { values } = xit();

    const scene = values.json("scene", {}, json_string);
</script>

<code>{JSON.stringify($scene, null, 4)}</code>