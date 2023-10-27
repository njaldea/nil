#include "gui.hpp"

#include "app/App.hpp"

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

#include <gen/nedit/messages/graph_update.pb.h>
#include <gen/nedit/messages/message.pb.h>
#include <gen/nedit/messages/node_info.pb.h>
#include <gen/nedit/messages/pin_info.pb.h>

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

    nil::service::tcp::Server server({
        .port = std::uint16_t(options.number("port")) //
    });

    std::mutex mutex;
    std::vector<std::function<void()>> actions;
    bool is_frozen = false;

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

    GLFWwindow* window = glfwCreateWindow( //
        1280,
        720,
        "Dear ImGui GLFW+OpenGL3 example",
        nullptr,
        nullptr
    );

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

    const ImGuiStyle& style = ImGui::GetStyle();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    auto* context = ax::NodeEditor::CreateEditor();
    App app;

    server.on_message( //
        [&actions,
         &mutex,
         &app,
         &is_frozen](const std::string&, const void* data, std::uint64_t count)
        {
            if (is_frozen)
            {
                return;
            }
            nil::nedit::proto::Message m;
            m.ParseFromArray(data, int(count));
            switch (m.type())
            {
                case nil::nedit::proto::type::Freeze:
                {
                    is_frozen = true;
                    return;
                }
                case nil::nedit::proto::type::PinInfo:
                {
                    nil::nedit::proto::PinInfo pin_info;
                    pin_info.ParseFromString(m.data());

                    const auto _ = std::unique_lock(mutex);
                    actions.push_back(
                        [&app,
                         label = pin_info.label(),
                         color = ImVec4(
                             pin_info.color().r(),
                             pin_info.color().g(),
                             pin_info.color().b(),
                             pin_info.color().a()
                         )]() mutable
                        {
                            app.add_pin_type({
                                std::move(label),
                                std::make_unique<FlowIcon>(std::move(color)) //
                            });                                              //
                        }
                    );
                    return;
                }
                case nil::nedit::proto::type::NodeInfo:
                {
                    nil::nedit::proto::NodeInfo node_info;
                    node_info.ParseFromString(m.data());

                    auto info = //
                        NodeInfo{
                            node_info.label(),
                            {node_info.inputs().begin(), node_info.inputs().end()},
                            {node_info.outputs().begin(), node_info.outputs().end()},
                        };

                    const auto _ = std::unique_lock(mutex);
                    actions.push_back(
                        [&app, info = std::move(info)]() mutable
                        {
                            app.add_node_type(std::move(info)); //
                        }
                    );
                    return;
                }
                default:
                {
                    return;
                }
            }
        }
    );

    std::thread comm([&server]() { server.run(); });

    static const auto draw = +[](GLFWwindow* w)
    {
        // Rendering code goes here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
    };

    static const auto resize = +[](GLFWwindow* w, int width, int height)
    {
        glViewport(0, 0, width, height);
        draw(w);
    };

    glfwSetFramebufferSizeCallback(window, resize);

    constexpr auto window_flag                    //
        = ImGuiWindowFlags_NoDecoration           //
        | ImGuiWindowFlags_NoMove                 //
        | ImGuiWindowFlags_NoScrollWithMouse      //
        | ImGuiWindowFlags_NoSavedSettings        //
        | ImGuiWindowFlags_NoBringToFrontOnFocus; //

    while (glfwWindowShouldClose(window) == 0)
    {
        for (const auto& action :
             [&]()
             {
                 const auto _ = std::unique_lock(mutex);
                 return std::move(actions);
             }())
        {
            action();
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::Begin("Content", nullptr, window_flag);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.WindowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.WindowRounding);

        app.render(context);

        ImGui::PopStyleVar(2);
        ImGui::End();

        ImGui::Begin("Panel");

        if (ImGui::Button("play"))
        {
            [&]()
            {
                nil::nedit::proto::Graph graph;
                for (const auto& n : app.nodes)
                {
                    auto* node = graph.add_nodes();
                    node->set_type(n.second->type);
                    for (auto idx = 0u; idx < n.second->pins_i.size(); ++idx)
                    {
                        if (n.second->pins_i[idx]->links.empty())
                        {
                            std::cout << "connections not complete" << std::endl;
                            return;
                        }
                        node->add_inputs(
                            app.links[*n.second->pins_i[idx]->links.begin()]->entry->id.Get()
                        );
                    }
                    for (auto idx = 0u; idx < n.second->pins_o.size(); ++idx)
                    {
                        node->add_outputs(n.second->pins_o[idx]->id.Get());
                    }
                }
                nil::nedit::proto::Message m;
                m.set_type(nil::nedit::proto::type::GraphUpdate);
                const auto d = m.SerializeAsString();
                server.publish(d.c_str(), d.size());
            }();
        }
        ImGui::Text("Pin Types");

        for (auto n = 0u; n < app.pin_type_count(); n++)
        {
            ImGui::Dummy(ImVec2(5, 0));
            ImGui::SameLine();
            ImGui::TextColored(app.pin_infos[n].icon->color, "%s", app.pin_infos[n].label.data());
        }
        ImGui::Text("Node Type (drag)");

        for (auto n = 0u; n < app.node_type_count(); n++)
        {
            ImGui::Dummy(ImVec2(5, 0));
            ImGui::SameLine();
            ImGui::Selectable(app.node_type_label(n));

            constexpr auto src_flags //
                = ImGuiDragDropFlags_SourceNoDisableHover
                | ImGuiDragDropFlags_SourceNoPreviewTooltip
                | ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
            if (ImGui::BeginDragDropSource(src_flags))
            {
                app.prepare_create(n);
                ImGui::EndDragDropSource();
            }
            else
            {
                app.confirm_create(n);
            }

            if (ImGui::BeginDragDropTarget())
            {
                ImGui::EndDragDropTarget();
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        resize(window, display_w, display_h);
    }

    ax::NodeEditor::DestroyEditor(context);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    server.stop();
    comm.join();

    return 0;
}
