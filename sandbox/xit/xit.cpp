#include "xit.hpp"

#include <nil/service/IService.hpp>
#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <nil/service/map.hpp>
#include <xit/messages/message.pb.h>

#include "codec.hpp"

#include <fstream>
#include <memory>
#include <unordered_map>
#include <variant>

namespace nil::xit
{
    template <typename T>
    struct Binding
    {
        Frame* frame = nullptr;
        std::string tag;
        T value;
        std::function<void(const T&)> on_change;
    };

    struct Frame
    {
        Core* x = nullptr;
        std::string id;
        std::filesystem::path path;

        using Binding_t = std::variant<Binding<std::int64_t>, Binding<std::string>>;
        std::unordered_map<std::string, Binding_t> bindings;
    };

    struct Core
    {
        nil::service::IService* service;
        std::unordered_map<std::string, Frame> frames;
    };

    void internal_set(Binding<std::int64_t>& binding, const nil::xit::proto::Binding& msg)
    {
        if (binding.value != msg.value_i64())
        {
            binding.value = msg.value_i64();
            binding.on_change(binding.value);
        }
    }

    void internal_set(Binding<std::string>& binding, const nil::xit::proto::Binding& msg)
    {
        if (binding.value != msg.value_str())
        {
            binding.value = msg.value_str();
            binding.on_change(binding.value);
        }
    }

    void msg_set(nil::xit::proto::Binding& msg, std::int64_t value)
    {
        msg.set_value_i64(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, std::string value)
    {
        msg.set_value_str(std::move(value));
    }

    template <typename T>
    void msg_set(nil::xit::proto::Binding& msg, const Binding<T>& binding)
    {
        msg_set(msg, binding.value);
    }

    void handle(
        Core& core,
        const nil::service::ID& id,
        const nil::xit::proto::MarkupRequest& request
    )
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            nil::xit::proto::MarkupResponse response;
            response.set_id(it->first);
            response.set_file(it->second.path);

            const auto header = nil::xit::proto::MessageType_MarkupResponse;
            auto payload = nil::service::concat(header, response);
            core.service->send(id, std::move(payload));
        }
        else
        {
            // error response
        }
    }

    void handle(
        Core& core,
        const nil::service::ID& id,
        const nil::xit::proto::BindingRequest& request
    )
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            nil::xit::proto::BindingResponse response;
            response.set_id(it->first);
            const auto& frame = it->second;
            for (const auto& [tag, binding] : frame.bindings)
            {
                auto* msg_binding = response.add_bindings();
                msg_binding->set_tag(tag);
                std::visit([msg_binding](const auto& v) { msg_set(*msg_binding, v); }, binding);
            }

            const auto header = nil::xit::proto::MessageType_BindingResponse;
            auto payload = nil::service::concat(header, response);
            core.service->send(id, std::move(payload));
        }
        else
        {
            // error response
        }
    }

    void handle(
        Core& core,
        const nil::service::ID& /* id */,
        const nil::xit::proto::BindingUpdate& msg
    )
    {
        auto it = core.frames.find(msg.id());
        if (it != core.frames.end())
        {
            auto binding_it = it->second.bindings.find(msg.binding().tag());
            if (binding_it != it->second.bindings.end())
            {
                if (msg.binding().has_value_i64())
                {
                    std::visit(
                        [&msg](auto& b) { internal_set(b, msg.binding()); },
                        binding_it->second
                    );
                }
                else if (msg.binding().has_value_str())
                {
                    std::visit(
                        [&msg](auto& b) { internal_set(b, msg.binding()); },
                        binding_it->second
                    );
                }
            }
        }
        else
        {
            // error response
        }
    }

    void handle(Core& core, const nil::service::ID& id, const nil::xit::proto::FileRequest& request)
    {
        nil::xit::proto::FileResponse response;
        response.set_target(request.target());

        std::fstream file(request.target());
        response.set_content(std::string( //
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        ));

        const auto header = nil::xit::proto::MessageType_FileResponse;
        auto payload = nil::service::concat(header, response);
        core.service->send(id, std::move(payload));
    }

    Frame& add_frame(Core& core, std::string id, std::filesystem::path path)
    {
        return core.frames.emplace(id, Frame{&core, id, std::move(path), {}}).first->second;
    }

    namespace impl
    {
        template <typename T, typename Callback>
        Binding<T>& bind(Frame& frame, std::string tag, T value, Callback on_change)
        {
            using type = Binding<T>;
            auto binding = type{&frame, tag, std::move(value), std::move(on_change)};
            return std::get<type>(frame.bindings.emplace(tag, std::move(binding)).first->second);
        }

        template <typename T>
        void post(Binding<T>& b, T v)
        {
            nil::xit::proto::BindingUpdate msg;
            msg.set_id(b.frame->id);
            auto* binding = msg.mutable_binding();
            binding->set_tag(b.tag);
            msg_set(*binding, std::move(v));

            const auto header = nil::xit::proto::MessageType_BindingUpdate;
            auto payload = nil::service::concat(header, msg);
            b.frame->x->service->publish(std::move(payload));
        }

        void install_on_message(Core& core)
        {
            auto make_handler = [ptr = &core](auto consume)
            {
                return [ptr, consume](const auto& id, const void* data, std::uint64_t size)
                { handle(*ptr, id, consume(data, size)); };
            };
            auto handlers            //
                = nil::service::map( //
                    nil::service::mapping(
                        nil::xit::proto::MessageType_MarkupRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::MarkupRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_BindingRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::BindingRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_FileRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::FileRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_BindingUpdate,
                        make_handler(&nil::service::consume<nil::xit::proto::BindingUpdate>)
                    )
                );
            core.service->on_message(std::move(handlers));
        }
    }

    Binding<std::string>& bind(
        Frame& frame,
        std::string tag,
        std::string value,
        std::function<void(const std::string&)> on_change
    )
    {
        return impl::bind(frame, std::move(tag), std::move(value), std::move(on_change));
    }

    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string tag,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change
    )
    {
        return impl::bind(frame, std::move(tag), value, std::move(on_change));
    }

    void post(Binding<std::int64_t>& b, std::int64_t v)
    {
        impl::post(b, v);
    }

    void post(Binding<std::string>& b, std::string v)
    {
        impl::post(b, std::move(v));
    }

    std::unique_ptr<Core, void (*)(Core*)> make_core(nil::service::IService& service)
    {
        using return_t = std::unique_ptr<Core, void (*)(Core*)>;
        constexpr auto deleter = [](Core* obj) { std::default_delete<Core>()(obj); };
        auto ret = return_t{new Core(&service, {}), deleter};
        impl::install_on_message(*ret);
        return ret;
    }
}
