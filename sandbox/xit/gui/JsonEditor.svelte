<script>
    import { getContext } from "svelte";
    import { JSONEditor } from 'vanilla-jsoneditor/standalone.js'
    import { json_string } from "@nil-/xit";

    /** @type import('./nil-xit').Xit */
    const { binding } = getContext("nil.xit");

    const buf_binding = binding.json('json_binding', {}, json_string);

    const json_editor = (d) => {
        const editor = new JSONEditor({
            target: d,
            props: {
                content: { json: $buf_binding },
                onChange: (updatedContent, previousContent, { contentErrors, patchResult }) => {
                    console.log(updatedContent);
                    if (updatedContent.json)
                    {
                        $buf_binding = updatedContent.json   
                    }
                    else if (updatedContent.text)
                    {
                        try
                        {
                            $buf_binding = JSON.parse(updatedContent.text)
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

<div style="display: contents" use:json_editor/>
