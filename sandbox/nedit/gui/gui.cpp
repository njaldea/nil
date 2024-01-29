#include "gui.hpp"
#include "../codec.hpp"

#include "app/App.hpp"
#include "app/Control.hpp"

#include <nil/service.hpp>

#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>

#include <gen/nedit/messages/control_update.pb.h>
#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/metadata.pb.h>
#include <gen/nedit/messages/state.pb.h>
#include <gen/nedit/messages/type.pb.h>

nil::cli::OptionInfo GUI::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port", .fallback = 1101})
        .build();
}

int GUI::run(const nil::cli::Options& options) const
{
    if (options.flag("help"))
    {
        options.help(std::cout);
        return 0;
    }

    bool loop = true;

    nil::service::TypedService server( //
        nil::service::make_service<nil::service::tcp::Server>({
            .port = std::uint16_t(options.number("port")) //
        })
    );

    glfwSetErrorCallback( //
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

    GLFWwindow* window
        = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);

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

    auto* context = ax::NodeEditor::CreateEditor();
    gui::App app;

    nil::nedit::proto::State info;

    server.on_message(
        nil::nedit::proto::message_type::State,
        [&app,
         &server,
         &info]                                                       //
        (const std::string&, const nil::nedit::proto::State& message) //
        {
            std::vector<std::function<void()>> tmp;
            tmp.emplace_back([&app]() mutable { app.reset(); });

            tmp.push_back([&info, message]() mutable { info = std::move(message); });

            for (const auto& pin : message.types().pins())
            {
                auto pin_info = gui::PinInfo{
                    pin.label(),
                    {ImVec4(pin.color().r(), pin.color().g(), pin.color().b(), pin.color().a())}
                };
                tmp.emplace_back( //
                    [&app, info = std::move(pin_info)]() mutable
                    { app.pin_infos.emplace_back(std::move(info)); }
                );
            }

            for (const auto& node : message.types().nodes())
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
                                [=, &server](gui::ID id)
                                {
                                    auto notifier = [&server](std::uint64_t nid, bool v)
                                    {
                                        namespace p = nil::nedit::proto;
                                        p::ControlUpdate msg;
                                        msg.set_id(nid);
                                        msg.set_b(v);
                                        server.publish(p::message_type::ControlUpdate, msg);
                                    };
                                    return std::make_unique<gui::ToggleControl>(
                                        std::move(id),
                                        value,
                                        std::move(notifier)
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
                                [=, &server](gui::ID id)
                                {
                                    auto notifier = [&server](std::uint64_t nid, std::int32_t v)
                                    {
                                        namespace p = nil::nedit::proto;
                                        p::ControlUpdate msg;
                                        msg.set_id(nid);
                                        msg.set_i(v);
                                        server.publish(p::message_type::ControlUpdate, msg);
                                    };
                                    return std::make_unique<gui::SpinboxControl>(
                                        std::move(id),
                                        value,
                                        min,
                                        max,
                                        std::move(notifier)
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
                                [=, &server](gui::ID id)
                                {
                                    auto notifier = [&server](std::uint64_t nid, float v)
                                    {
                                        namespace p = nil::nedit::proto;
                                        p::ControlUpdate msg;
                                        msg.set_id(nid);
                                        msg.set_f(v);
                                        server.publish(p::message_type::ControlUpdate, msg);
                                    };
                                    return std::make_unique<gui::SliderControl>(
                                        std::move(id),
                                        value,
                                        min,
                                        max,
                                        std::move(notifier)
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
                                [=, &server](gui::ID id)
                                {
                                    auto notifier
                                        = [&server](std::uint64_t nid, const std::string& v)
                                    {
                                        namespace p = nil::nedit::proto;
                                        p::ControlUpdate msg;
                                        msg.set_id(nid);
                                        msg.set_s(v);
                                        server.publish(p::message_type::ControlUpdate, msg);
                                    };
                                    return std::make_unique<gui::TextControl>(
                                        std::move(id),
                                        value,
                                        std::move(notifier)
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
                                [=, &server](gui::ID id)
                                {
                                    auto notifier
                                        = [&server](std::uint64_t nid, const std::string& v)
                                    {
                                        namespace p = nil::nedit::proto;
                                        p::ControlUpdate msg;
                                        msg.set_id(nid);
                                        msg.set_s(v);
                                        server.publish(p::message_type::ControlUpdate, msg);
                                    };
                                    return std::make_unique<gui::ComboBoxControl>(
                                        std::move(id),
                                        value,
                                        selection,
                                        std::move(notifier)
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
                tmp.emplace_back( //
                    [&app, node_info = std::move(node_info)]() mutable
                    { app.node_infos.emplace_back(std::move(node_info)); }
                );
            }

            tmp.emplace_back(
                [&info, &app]()
                {
                    nil::nedit::proto::Metadata m;
                    m.ParseFromString(info.metadata());
                    for (const auto& node : m.nodes())
                    {
                        app.load_node(
                            node.id(),
                            node.type(),
                            {node.inputs().begin(), node.inputs().end()},
                            {node.outputs().begin(), node.outputs().end()},
                            {node.controls().begin(), node.controls().end()}
                        );
                        ax::NodeEditor::SetNodePosition(node.id(), ImVec2{node.x(), node.y()});
                    }
                    for (const auto& link : m.links())
                    {
                        app.load_link(link.id(), link.input(), link.output());
                    }
                }
            );

            // load the graph here if it is available;
            const auto _ = std::unique_lock(app.mutex);
            app.before_render = std::move(tmp);
        }
    );

    std::thread comm([&server]() { server.run(); });

    static constexpr auto draw = +[](GLFWwindow* w)
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
    };

    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* w, auto width, auto height)
        {
            glViewport(0, 0, width, height);
            draw(w);
        }
    );

    constexpr auto window_flag               //
        = ImGuiWindowFlags_NoDecoration      //
        | ImGuiWindowFlags_NoMove            //
        | ImGuiWindowFlags_NoScrollWithMouse //
        | ImGuiWindowFlags_NoSavedSettings   //
        | ImGuiWindowFlags_NoBringToFrontOnFocus;

    const auto load = [&]()
    {
        auto* graph = info.mutable_graph();
        graph->clear_nodes();

        nil::nedit::proto::Metadata metadata;
        for (const auto& [id, n] : app.nodes)
        {
            // for graph
            {
                auto* node = graph->add_nodes();
                node->set_id(id);
                node->set_type(n->type);
                for (const auto& pin : n->pins_i)
                {
                    const auto& links = app.pins.at(pin.id.value).links;
                    if (links.empty())
                    {
                        std::cout << "connections not complete" << std::endl;
                        return;
                    }
                    node->add_inputs((*links.begin())->entry->id.value);
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
            // for metadata
            {
                auto* node = metadata.add_nodes();
                const auto pos = ax::NodeEditor::GetNodePosition(id);
                node->set_id(id);
                node->set_type(n->type);
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
                node->set_x(pos.x);
                node->set_y(pos.y);
            }
        }

        for (const auto& [id, link] : app.links)
        {
            auto* meta_link = metadata.add_links();
            meta_link->set_id(id);
            meta_link->set_input(link->entry->id.value);
            meta_link->set_output(link->exit->id.value);
        }
        info.set_metadata(metadata.SerializeAsString());
        server.publish(nil::nedit::proto::message_type::Load, info);
    };

    while (glfwWindowShouldClose(window) == 0 && loop)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::Begin("Content", nullptr, window_flag);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ImGui::GetStyle().WindowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ImGui::GetStyle().WindowRounding);

        app.render(*context);

        ImGui::PopStyleVar(2);
        ImGui::End();

        ImGui::Begin("Panel");
        ImGui::PushItemWidth(1.0f);

        // will be turned into load file
        if (ImGui::Button("force-load"))
        {
            const auto _ = std::unique_lock(app.mutex);
            app.after_render.push_back(load);
        }

        app.render_panel();

        // if (app.has_changed()) {
        //     send an update
        // }

        ImGui::PopItemWidth();
        ImGui::End();

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

    server.stop();
    comm.join();

    return 0;
}
