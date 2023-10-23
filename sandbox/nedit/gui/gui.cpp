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

nil::cli::OptionInfo GUI::options() const
{
    return nil::cli::Builder()
        .flag("help", {.skey = 'h', .msg = "this help"})
        .number("port", {.skey = 'p', .msg = "port"})
        .build();
}

int GUI::run(const nil::cli::Options& options) const
{
    (void)options;
    nil::service::tcp::Server server({
        .port = std::uint16_t(options.number("port")) //
    });

    std::mutex mutex;
    std::vector<std::function<void()>> actions;

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
        [&actions, &mutex, &app](const std::string&, const void* data, std::uint64_t count)
        {
            const auto message = std::string_view(static_cast<const char*>(data), count);
            if (message.starts_with("node:"))
            {
                const auto label = std::string(message.data() + 5, message.size() - 5);
                const auto separator = std::find(label.begin(), label.end(), '-');
                if (separator == label.end())
                {
                    return;
                }

                const auto process = [&](const std::string& text)
                {
                    std::vector<std::uint32_t> items;
                    std::istringstream ss(text);
                    std::string token;
                    while (std::getline(ss, token, ','))
                    {
                        items.push_back(std::uint32_t(std::stoul(token)));
                    }
                    return items;
                };

                auto info = NodeInfo{
                    std::move(label),
                    process({label.begin(), separator}),
                    process({separator + 1, label.end()})
                };

                const auto _ = std::unique_lock(mutex);
                actions.push_back(
                    [&app, info = std::move(info)]() mutable
                    {
                        app.add_node_type(std::move(info)); //
                    }
                );
            }
            if (message.starts_with("pin:"))
            {
                const auto dd = std::string(message.data() + 4, message.size() - 4);

                ImVec4 color = [&]()
                {
                    std::istringstream ss(dd);
                    std::string token;
                    std::getline(ss, token, ',');
                    const auto c1 = std::stof(token);
                    std::getline(ss, token, ',');
                    const auto c2 = std::stof(token);
                    std::getline(ss, token, ',');
                    const auto c3 = std::stof(token);
                    std::getline(ss, token, ',');
                    const auto c4 = std::stof(token);
                    return ImVec4(c1, c2, c3, c4);
                }();
                const auto _ = std::unique_lock(mutex);
                actions.push_back(
                    [&app, label = std::move(dd), color = std::move(color)]() mutable
                    {
                        app.add_pin_type({
                            std::move(label),
                            std::make_unique<FlowIcon>(std::move(color)) //
                        });                                              //
                    }
                );
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

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(1280, 720));
        ImGui::Begin("Content", nullptr, window_flag);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.WindowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.WindowRounding);

        app.render(context);

        ImGui::PopStyleVar(2);
        ImGui::End();

        ImGui::Begin("Panel");
        ImGui::Text("Pin Types");

        for (auto n = 0u; n < app.pin_type_count(); n++)
        {
            ImGui::TextColored(app.pin_infos[n].icon->color, "%d", n);
        }
        ImGui::Text("Node Type (drag)");

        for (auto n = 0u; n < app.node_type_count(); n++)
        {
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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
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
