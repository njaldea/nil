<script>
    import { createJSONEditor } from 'vanilla-jsoneditor/standalone.js'
    import { xit, json_string } from "@nil-/xit";

    const { values } = xit();

    const buf_value = values.json('value', {}, json_string);

    const json_editor = (target) => {
        let is_notified = true;
        const editor = createJSONEditor({
            target,
            props: {
                content: { json: null },
                onChange: (updatedContent, previousContent, { contentErrors, patchResult }) => {
                    if (is_notified) {
                        return;
                    }
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
        const unsub = buf_value.subscribe((v) => {
            is_notified = true;
            editor.set({ json: v });
            is_notified = false;
        });
        return {
            destroy: () => {
                unsub();
                editor.destroy();
            }
        };
    };
</script>

<svelte:head>
    <title>nil - xit - editor</title>
</svelte:head>

<div style="display: contents" use:json_editor></div>