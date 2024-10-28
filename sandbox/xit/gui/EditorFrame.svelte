<script>
    import { createJSONEditor } from 'vanilla-jsoneditor/standalone.js'
    import { xit, json_string } from "@nil-/xit";

    const { values } = xit();

    const buf_value = values.json('scene', {}, json_string);

    const json_editor = (target) => {
        const editor = createJSONEditor({
            target,
            props: {
                content: { json: $buf_value },
                onChange: (updatedContent, previousContent, { contentErrors, patchResult }) => {
                    if ("json" in updatedContent)
                    {
                        $buf_value = updatedContent.json;
                    }
                    else if ("text" in updatedContent)
                    {
                        try
                        {
                            $buf_value = JSON.parse(updatedContent.text)
                        }
                        catch (e)
                        {
                            console.log(e);
                        }
                    }
                }
            }
        });
        return { destroy: () => editor.destroy() };
    };
</script>

<svelte:head>
    <title>nil - xit - editor</title>
</svelte:head>

<div style="display: contents" use:json_editor></div>