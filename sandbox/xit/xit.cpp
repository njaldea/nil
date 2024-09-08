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
        Frame& frame; // NOLINT
        std::string tag;
        T value;
        std::function<void(const T&)> on_change;
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

    void msg_set(nil::xit::proto::Binding& msg, const Binding<std::int64_t>& binding)
    {
        msg.set_value_i64(binding.value);
    }

    void msg_set(nil::xit::proto::Binding& msg, const Binding<std::string>& binding)
    {
        msg.set_value_str(binding.value);
    }

    struct Frame
    {
        nil::service::IService& service; // NOLINT
        std::string id;
        std::filesystem::path path;
        std::unordered_map<std::string, std::variant<Binding<std::int64_t>, Binding<std::string>>>
            bindings;
    };

    struct Xit
    {
        explicit Xit(nil::service::IService& init_service)
            : service(init_service)
        {
            auto make_handler = [&](auto consume)
            {
                return [&, consume](const auto& id, const void* data, std::uint64_t size)
                { handle(id, consume(data, size)); };
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
            service.on_message(std::move(handlers));
        }

        void handle(const nil::service::ID& id, const nil::xit::proto::MarkupRequest& request)
        {
            const auto it = frames.find(request.id());
            if (it != frames.end())
            {
                nil::xit::proto::MarkupResponse response;
                response.set_id(it->first);
                response.set_file(it->second.path);

                const auto header = nil::xit::proto::MessageType_MarkupResponse;
                auto payload = nil::service::concat(header, response);
                service.send(id, std::move(payload));
            }
            else
            {
                // error response
            }
        }

        void handle(const nil::service::ID& id, const nil::xit::proto::BindingRequest& request)
        {
            const auto it = frames.find(request.id());
            if (it != frames.end())
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
                service.send(id, std::move(payload));
            }
            else
            {
                // error response
            }
        }

        void handle(const nil::service::ID& /* id */, const nil::xit::proto::BindingUpdate& msg)
        {
            auto it = frames.find(msg.id());
            if (it != frames.end())
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
                    if (msg.binding().has_value_str())
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

        void handle(const nil::service::ID& id, const nil::xit::proto::FileRequest& request)
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
            service.send(id, std::move(payload));
        }

        nil::service::IService& service; // NOLINT
        std::unordered_map<std::string, Frame> frames;
    };

    Frame& frame(Xit& x, std::string id, std::filesystem::path path)
    {
        return x.frames.emplace(id, Frame{x.service, id, std::move(path), {}}).first->second;
    }

    Binding<std::string>& bind(
        Frame& f,
        std::string tag,
        std::string value,
        std::function<void(const std::string&)> on_change
    )
    {
        using type = Binding<std::string>;
        auto binding = type{f, tag, std::move(value), std::move(on_change)};
        return std::get<type>(f.bindings.emplace(tag, std::move(binding)).first->second);
    }

    Binding<std::int64_t>& bind(
        Frame& f,
        std::string tag,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change
    )
    {
        using type = Binding<std::int64_t>;
        auto binding = type{f, tag, value, std::move(on_change)};
        return std::get<type>(f.bindings.emplace(tag, std::move(binding)).first->second);
    }

    void post(Binding<std::int64_t>& b, std::int64_t v)
    {
        nil::xit::proto::BindingUpdate msg;
        msg.set_id("id-1");
        auto* binding = msg.mutable_binding();
        binding->set_tag(b.tag);
        binding->set_value_i64(v);
        b.frame.service.publish(
            nil::service::concat(nil::xit::proto::MessageType_BindingUpdate, msg)
        );
    }

    void post(Binding<std::string>& b, std::string v)
    {
        nil::xit::proto::BindingUpdate msg;
        msg.set_id(b.frame.id);
        auto* binding = msg.mutable_binding();
        binding->set_tag(b.tag);
        binding->set_value_str(std::move(v));
        b.frame.service.publish(
            nil::service::concat(nil::xit::proto::MessageType_BindingUpdate, msg)
        );
    }

    void Deleter::operator()(Xit* obj) const
    {
        std::default_delete<Xit>()(obj);
    }

    std::unique_ptr<Xit, Deleter> make_xit(nil::service::IService& service)
    {
        return std::unique_ptr<Xit, Deleter>(new Xit(service));
    }
}
