#include "gui.hpp"
#include "../codec.hpp"

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
#include <gen/nedit/messages/node_info.pb.h>
#include <gen/nedit/messages/pin_info.pb.h>
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

    nil::service::TypedService server(               //
        std::make_unique<nil::service::tcp::Server>( //
            nil::service::tcp::Server::Options{
                .port = std::uint16_t(options.number("port")) //
            }
        )
    );

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

    server.on_message(
        nil::nedit::proto::message_type::Freeze,
        // [TODO] find a way to not care about arguments if possible
        [&is_frozen] //
        (const std::string&, const std::string&)
        {
            is_frozen = true; //
        }
    );

    server.on_message(
        nil::nedit::proto::message_type::PinInfo,
        [&is_frozen,
         &mutex,
         &actions,
         &app] //
        (const std::string&, const nil::nedit::proto::PinInfo& message)
        {
            if (is_frozen)
            {
                return;
            }
            auto info = PinInfo{
                message.label(),
                {ImVec4(
                    message.color().r(),
                    message.color().g(),
                    message.color().b(),
                    message.color().a()
                )}
            };
            const auto _ = std::unique_lock(mutex);
            actions.emplace_back(                                 //
                [&app, info = std::move(info)]                    //
                () mutable { app.add_pin_type(std::move(info)); } //
            );
        }
    );

    server.on_message(
        nil::nedit::proto::message_type::NodeInfo,
        [&is_frozen,
         &mutex,
         &actions,
         &app] //
        (const std::string&, const nil::nedit::proto::NodeInfo& message)
        {
            if (is_frozen)
            {
                return;
            }

            auto info = NodeInfo{
                message.label(),
                {message.inputs().begin(), message.inputs().end()},
                {message.outputs().begin(), message.outputs().end()},
            };

            const auto _ = std::unique_lock(mutex);
            actions.emplace_back(
                [&app, info = std::move(info)]() mutable
                {
                    app.add_node_type(std::move(info)); //
                }
            );
        }
    );

    std::thread comm([&server]() { server.run(); });

    static const auto draw = +[](GLFWwindow* w)
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

    constexpr auto window_flag                    //
        = ImGuiWindowFlags_NoDecoration           //
        | ImGuiWindowFlags_NoMove                 //
        | ImGuiWindowFlags_NoScrollWithMouse      //
        | ImGuiWindowFlags_NoSavedSettings        //
        | ImGuiWindowFlags_NoBringToFrontOnFocus; //

    while (glfwWindowShouldClose(window) == 0)
    {
        for (const auto& action :
             [&actions, &mutex]()
             {
                 std::vector<std::function<void()>> r;
                 {
                     const auto _ = std::unique_lock(mutex);
                     std::swap(r, actions);
                 }
                 return r;
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
                    for (const auto& pin : n.second->pins_i)
                    {
                        if (pin->links.empty())
                        {
                            std::cout << "connections not complete" << std::endl;
                            return;
                        }
                        node->add_inputs(app.links[*pin->links.begin()]->entry->id.Get());
                    }
                    for (const auto& pin : n.second->pins_o)
                    {
                        node->add_outputs(pin->id.Get());
                    }
                }
                server.publish(nil::nedit::proto::message_type::GraphUpdate, graph);
            }();
        }
        ImGui::Text("Pin Types");

        for (auto n = 0u; n < app.pin_type_count(); n++)
        {
            ImGui::Dummy(ImVec2(5, 0));
            ImGui::SameLine();
            ImGui::TextColored(app.pin_infos[n].icon.color, "%s", app.pin_infos[n].label.data());
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
        glViewport(0, 0, display_w, display_h);
        draw(window);
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
