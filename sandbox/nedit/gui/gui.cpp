#include "gui.hpp"
#include "../codec.hpp" // IWYU pragma: keep
#include "app/App.hpp"
#include "app/Control.hpp"

#include <nil/dev.hpp>
#include <nil/gate.hpp>
#include <nil/service.hpp>

#include <gen/nedit/messages/metadata.pb.h>
#include <gen/nedit/messages/node_state.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <GLFW/glfw3.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace
{

    constexpr auto window_flag                   //
        = ImGuiWindowFlags_NoDecoration          //
        | ImGuiWindowFlags_NoMove                //
        | ImGuiWindowFlags_NoScrollWithMouse     //
        | ImGuiWindowFlags_NoSavedSettings       //
        | ImGuiWindowFlags_NoBringToFrontOnFocus //
        | ImGuiWindowFlags_NoResize;

    constexpr auto draw = +[](GLFWwindow* w)
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
    };

    void update_metadata(nil::nedit::proto::Metadata& metadata, std::uint64_t id)
    {
        auto* node = metadata.add_nodes();
        const auto pos = ax::NodeEditor::GetNodePosition(id);
        node->set_id(id);
        node->set_x(pos.x);
        node->set_y(pos.y);
    }

    void load(const gui::App& app, nil::nedit::proto::State& info)
    {
        auto* graph = info.mutable_graph();
        graph->clear_nodes();
        graph->clear_links();

        nil::nedit::proto::Metadata metadata;

        for (const auto& [id, link] : app.links)
        {
            auto* l = graph->add_links();
            l->set_id(id);
            l->set_input(link->entry->id.value);
            l->set_output(link->exit->id.value);
            l->set_type(link->entry->type != 0 ? link->entry->type : link->exit->type);
        }
        for (const auto& [id, n] : app.nodes)
        {
            // for graph
            {
                auto* node = graph->add_nodes();
                node->set_id(id);
                node->set_type(n->type);
                if (n->type == 0 || n->type == 1)
                {
                    node->set_alias(n->pins_i[0].alias);
                }
                else
                {
                    node->set_alias(0u);
                }
                for (const auto& pin : n->pins_i)
                {
                    node->add_inputs(pin.id.value);
                }
                for (const auto& pin : n->pins_o)
                {
                    node->add_outputs(pin.id.value);
                }
                for (const auto& control : n->controls)
                {
                    node->add_controls(control->id.value);
                }
            }
            update_metadata(metadata, id);
        }
        info.set_metadata(metadata.SerializeAsString());
    }

    auto process_state(
        nil::service::IService& service,
        gui::App& app,
        nil::nedit::proto::State& info,
        const nil::nedit::proto::State& new_info
    )
    {
        std::vector<std::function<void()>> tmp;
        tmp.emplace_back([&app]() { app.reset(); });
        tmp.emplace_back([&info, m = new_info]() mutable { info = std::move(m); });

        // pins
        tmp.emplace_back(
            [&info, &app]()
            {
                for (const auto& pin : info.types().pins())
                {
                    app.pin_infos.push_back({
                        pin.label(),
                        ImVec4(
                            pin.color().r(),
                            pin.color().g(),
                            pin.color().b(),
                            pin.color().a()
                        ) //
                    });
                }
            }
        );

        // nodes
        tmp.emplace_back(
            [&info, &app, &service]()
            {
                for (const auto& node : info.types().nodes())
                {
                    auto node_info = gui::NodeInfo{
                        node.label(),
                        {node.inputs().begin(), node.inputs().end()},
                        {node.outputs().begin(), node.outputs().end()},
                        {}
                    };

                    using Control = nil::nedit::proto::State::Types::Node::Control;
                    for (const auto& control : node.controls())
                    {
                        switch (control.control_case())
                        {
                            case Control::ControlCase::kToggle:
                            {
                                const auto& toggle = control.toggle();
                                const auto value = toggle.value();
                                node_info.controls.emplace_back(
                                    [=, &service](gui::ID id) {
                                        return std::make_unique<gui::ToggleControl>(
                                            service,
                                            std::move(id),
                                            value
                                        );
                                    }
                                );
                                break;
                            }
                            case Control::ControlCase::kSpinbox:
                            {
                                const auto& spinbox = control.spinbox();
                                const auto value = spinbox.value();
                                const auto min = spinbox.min();
                                const auto max = spinbox.max();
                                node_info.controls.emplace_back(
                                    [=, &service](gui::ID id) {
                                        return std::make_unique<gui::SpinboxControl>(
                                            service,
                                            std::move(id),
                                            value,
                                            min,
                                            max
                                        );
                                    }
                                );
                                break;
                            }
                            case Control::ControlCase::kSlider:
                            {
                                const auto& slider = control.slider();
                                const auto value = slider.value();
                                const auto min = slider.min();
                                const auto max = slider.max();
                                node_info.controls.emplace_back(
                                    [=, &service](gui::ID id) {
                                        return std::make_unique<gui::SliderControl>(
                                            service,
                                            std::move(id),
                                            value,
                                            min,
                                            max
                                        );
                                    }
                                );
                                break;
                            }
                            case Control::ControlCase::kText:
                            {
                                const auto& text = control.text();
                                const auto& value = text.value();
                                node_info.controls.emplace_back(
                                    [=, &service](gui::ID id) {
                                        return std::make_unique<gui::TextControl>(
                                            service,
                                            std::move(id),
                                            value
                                        );
                                    }
                                );
                                break;
                            }
                            case Control::ControlCase::kCombobox:
                            {
                                const auto& combobox = control.combobox();
                                const auto& value = combobox.value();
                                const auto& selection = std::vector<std::string>(
                                    combobox.selection().begin(),
                                    combobox.selection().end()
                                );
                                node_info.controls.emplace_back(
                                    [=, &service](gui::ID id) {
                                        return std::make_unique<gui::ComboBoxControl>(
                                            service,
                                            std::move(id),
                                            value,
                                            selection
                                        );
                                    }
                                );
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                    }
                    app.node_infos.emplace_back(std::move(node_info));
                }
            }
        );

        tmp.emplace_back(
            [&info, &app]()
            {
                for (const auto& node : info.graph().nodes())
                {
                    app.load_node(
                        node.id(),
                        node.type(),
                        {node.inputs().begin(), node.inputs().end()},
                        {node.outputs().begin(), node.outputs().end()},
                        {node.controls().begin(), node.controls().end()}
                    );
                }
                for (const auto& link : info.graph().links())
                {
                    app.load_link(link.id(), link.input(), link.output());
                }

                nil::nedit::proto::Metadata m;
                m.ParseFromString(info.metadata());
                for (const auto& node : m.nodes())
                {
                    ax::NodeEditor::SetNodePosition(node.id(), ImVec2{node.x(), node.y()});
                }
            }
        );

        return tmp;
    }

    int run(const nil::clix::Options& options)
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }

        nil::service::tcp::Client service(
            {.host = "127.0.0.1", .port = std::uint16_t(options.number("port"))}
        );

        glfwSetErrorCallback(
            [](int error, const char* description)
            { std::cerr << "GLFW Error " << error << ": " << description << std::endl; }
        );

        if (glfwInit() == 0)
        {
            return 1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

        GLFWwindow* window = glfwCreateWindow(1280, 720, "nedit/gate POC", nullptr, nullptr);

        if (window == nullptr)
        {
            return 1;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // const ImGuiIO& io = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        gui::App app;
        nil::nedit::proto::State info;

        service.on_message(nil::service::map(
            nil::service::mapping(
                nil::nedit::proto::message_type::State,
                [&](const nil::nedit::proto::State& message)
                {
                    auto tmp = process_state(service, app, info, message);

                    // load the graph here if it is available;
                    const auto _ = std::unique_lock(app.mutex);
                    for (auto&& cb : tmp)
                    {
                        app.before_render.emplace_back(std::move(cb));
                    }
                }
            ),
            nil::service::mapping(
                nil::nedit::proto::message_type::NodeState,
                [&](const nil::nedit::proto::NodeState& message)
                {
                    const auto _ = std::unique_lock(app.mutex);
                    app.before_render.emplace_back( //
                        [&app, id = message.id(), activated = message.active()]()
                        { app.nodes.at(id)->activated = activated; }
                    );
                }
            )
        ));

        std::string path = options.param("file");
        bool loaded = true;

        service.on_connect( //
            [&loaded, &app]()
            { app.before_render.emplace_back([&loaded]() { loaded = false; }); } //
        );

        std::thread comm([&service]() { service.run(); });

        glfwSetFramebufferSizeCallback(
            window,
            [](GLFWwindow* w, auto width, auto height)
            {
                glViewport(0, 0, width, height);
                draw(w);
            }
        );

        auto* context = ax::NodeEditor::CreateEditor();
        while (glfwWindowShouldClose(window) == 0)
        {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            const auto* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);

            ImGui::PushID(1);
            ImGui::Begin("Content", nullptr, window_flag);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ImGui::GetStyle().WindowBorderSize);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ImGui::GetStyle().WindowRounding);
            app.render(*context);
            ImGui::PopStyleVar(2);
            ImGui::End();
            ImGui::PopID();

            ImGui::PushID(2);
            ImGui::Begin("Panel");

            ImGui::PushItemWidth(std::max(ImGui::GetWindowWidth() - 20.0f, 300.f));
            ImGui::InputText("###file", &path);
            ImGui::PopItemWidth();
            ImGui::BeginDisabled(!app.allow_editing);
            if ((ImGui::Button("load") || !loaded) && std::filesystem::exists(path))
            {
                try
                {
                    std::ifstream file(path, std::ios::binary);
                    nil::nedit::proto::State tmp;
                    tmp.ParseFromIstream(&file);
                    app.changed = true;
                    loaded = true;
                    service.publish(
                        nil::service::concat(nil::nedit::proto::message_type::State, tmp)
                    );
                }
                catch (const std::exception& e)
                {
                    nil::log(e.what());
                }
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(app.allow_editing);
            if (ImGui::Button("save"))
            {
                const auto _ = std::unique_lock(app.mutex);
                app.before_render.emplace_back(
                    [&]()
                    {
                        nil::nedit::proto::Metadata metadata;
                        for (const auto& node : app.nodes)
                        {
                            update_metadata(metadata, node.first);
                        }
                        info.set_metadata(metadata.SerializeAsString());
                        try
                        {
                            std::ofstream file(path, std::ios::binary);
                            // TODO: make sure that the path is valid:
                            //  -  not a directory
                            //  -  ---
                            info.SerializeToOstream(&file);
                        }
                        catch (const std::exception& e)
                        {
                            nil::log(e.what());
                        }
                    }
                );
            }
            ImGui::EndDisabled();

            if (ImGui::Checkbox("allow editing", &app.allow_editing))
            {
                if (app.allow_editing)
                {
                    service.publish(nil::nedit::proto::message_type::Pause);
                }
                else
                {
                    if (app.changed)
                    {
                        const auto _ = std::unique_lock(app.mutex);
                        app.after_render.emplace_back(
                            [&]()
                            {
                                load(app, info);
                                service.publish(nil::service::concat(
                                    nil::nedit::proto::message_type::State,
                                    info
                                ));
                                service.publish(nil::nedit::proto::message_type::Play);
                                service.publish(nil::nedit::proto::message_type::Run);
                            }
                        );
                        app.changed = false;
                    }
                    else
                    {
                        service.publish(nil::nedit::proto::message_type::Play);
                        service.publish(nil::nedit::proto::message_type::Run);
                    }
                }
            }

            app.render_panel();

            ImGui::End();
            ImGui::PopID();

            ImGui::Render();
            int display_w = 0;
            int display_h = 0;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            draw(window);
        }

        ax::NodeEditor::DestroyEditor(context);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();

        service.stop();
        comm.join();

        return 0;
    }
}

namespace GUI
{
    void apply(nil::clix::Node& node)
    {
        node.flag("help", {.skey = 'h', .msg = "this help"});
        node.number("port", {.skey = 'p', .msg = "port", .fallback = 1101});
        node.param("file", {.skey = 'f', .msg = "load file on boot", .fallback = ""});
        node.runner(run);
    }
}
