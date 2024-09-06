<script lang="ts">
    import { bundle } from "./bundle";
    import { writable, type Writable } from "svelte/store";

    import { Service, service_fetch, header, concat } from "$lib/Service";
    import { nil_wix_proto } from "$lib/proto";

    let { port, host } = $props<{ port: number; host: string; }>();

    const binding_fetch = async (service: Service) => {
        const bindings = await service_fetch(
            { host, port },
            header(nil_wix_proto.MessageType.MessageType_BindingRequest),
            (tag, data) => {
                if (tag === nil_wix_proto.MessageType.MessageType_BindingResponse)
                {
                    return nil_wix_proto.BindingResponse.decode(data);
                }
                throw "err";
            }
        );
        const contexts = [];
        for (const group of bindings.info) {
            const context = new Map<string, Writable<number | string>>();
            for (const b of group.bindings) {
                if (b.valueI64 != null)
                {
                    const w = writable(b.valueI64);
                    context.set(b.tag, w);
                    w.subscribe(v => service.publish(
                        concat([
                            header(nil_wix_proto.MessageType.MessageType_BindingUpdate),
                            nil_wix_proto.Binding.encode({ tag: b.tag, valueI64: v }).finish()
                        ])
                    ));
                }
                else if (b.valueStr)
                {
                    const w = writable(b.valueStr);
                    context.set(b.tag, w);
                    w.subscribe(v => service.publish(
                        concat([
                            header(nil_wix_proto.MessageType.MessageType_BindingUpdate),
                            nil_wix_proto.Binding.encode({ tag: b.tag, valueStr: v }).finish()
                        ])
                    ));
                }
            }
            contexts.push(context);
        }
        return contexts;
    };

    const bundle_step = async () => {
        const service = new Service({ host, port });
        service.on_connect(() => console.log('conected'));
        service.on_disconnect(() => console.log('disconected'));
        const m = await bundle({ host, port });
        const bindings = await binding_fetch(service);
        const action = m.action(m.components, bindings);;
        return (div: HTMLDivElement) => {
            const ret = action(div);
            service.start();
            return {
                destroy: () => {
                    service.stop();
                    ret.destroy();
                }
            };
        };
    };
</script>

{#await bundle_step()}
    Loading...
{:then action}
    <div use:action></div>
{:catch e}
    {(() => {
        console.log(e.stack);
        return "Something went wrong...";
    })()}
{/await}
