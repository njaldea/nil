#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include "app/App.hpp"

#include <iostream>

#include <nil/cli.hpp>
#include <nil/cli/nodes/Help.hpp>
#include <nil/service.hpp>

#include <functional>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>

struct EXT: nil::cli::Command
{
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag("help", {.skey = 'h', .msg = "this help"})
            .number("port", {.skey = 'p', .msg = "port"})
            .build();
    }

    int run(const nil::cli::Options& options) const override
    {
        if (options.flag("help"))
        {
            options.help(std::cout);
            return 0;
        }
        nil::service::tcp::Client client({
            .host = "127.0.0.1",                          //
            .port = std::uint16_t(options.number("port")) //
        });
        client.on_connect(
            [&](const std::string& id)
            {
                client.send(id, "pin:1,0.5,0.5,1", sizeof("pin:1,0.5,0.5,1"));
                client.send(id, "pin:0.5,1,0.5,1", sizeof("pin:0.5,1,0.5,1"));
                client.send(id, "node:0-0", sizeof("node:0-0"));
                client.send(id, "node:0-1", sizeof("node:0-1"));
                client.send(id, "node:1-1", sizeof("node:1-1"));
                client.send(id, "node:1-0", sizeof("node:1-0"));
                client.send(id, "node:0,1-1,0", sizeof("node:0,1-1,0"));
            }
        );

        while (true)
        {
            std::thread t1([&]() { client.run(); });
            std::string message;
            while (std::getline(std::cin, message))
            {
                if (message == "reconnect")
                {
                    break;
                }
                client.publish(message.data(), message.size());
            }
            client.stop();
            t1.join();
            client.restart();
        }
        return 0;
    }
};

struct GUI: nil::cli::Command
{
    nil::cli::OptionInfo options() const override
    {
        return nil::cli::Builder()
            .flag("help", {.skey = 'h', .msg = "this help"})
            .number("port", {.skey = 'p', .msg = "port"})
            .build();
    }

    int run(const nil::cli::Options& options) const override
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
                        std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__
                                  << std::endl;
                        return;
                    }

                    std::vector<std::uint32_t> inputs;
                    {
                        const auto i = std::string(label.begin(), separator);
                        std::istringstream ss(i);
                        std::string token;
                        while (std::getline(ss, token, ','))
                        {
                            inputs.push_back(std::uint32_t(std::stoul(token)));
                        }
                    }

                    std::vector<std::uint32_t> outputs;
                    {
                        const auto o = std::string(separator + 1, label.end());
                        std::istringstream ss(o);
                        std::string token;
                        while (std::getline(ss, token, ','))
                        {
                            outputs.push_back(std::uint32_t(std::stoul(token)));
                        }
                    }

                    const auto _ = std::unique_lock(mutex);
                    actions.push_back( //
                        [&app,
                         info = NodeInfo{std::move(label), std::move(inputs), std::move(outputs)}]()
                        { app.add_node_type(info); }
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
                    actions.push_back( //
                        [&app, info = PinInfo{std::move(dd), std::move(color)}]()
                        { app.add_pin_type(info); }
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

        while (glfwWindowShouldClose(window) == 0)
        {
            std::vector<std::function<void()>> aaa;
            {
                const auto _ = std::unique_lock(mutex);
                aaa = std::move(actions);
            }
            for (const auto& a : aaa)
            {
                a();
            }

            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(1280, 720));
            ImGui::Begin(
                "Content",
                nullptr,
                ImGuiWindowFlags_NoDecoration                //
                    | ImGuiWindowFlags_NoMove                //
                    | ImGuiWindowFlags_NoScrollWithMouse     //
                    | ImGuiWindowFlags_NoSavedSettings       //
                    | ImGuiWindowFlags_NoBringToFrontOnFocus //
            );
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, style.WindowBorderSize);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, style.WindowRounding);

            app.render(context);

            ImGui::PopStyleVar(2);
            ImGui::End();

            ImGui::Begin("Panel");
            ImGui::Text("Pin Types");

            for (auto n = 0u; n < app.pin_type_count(); n++)
            {
                ImGui::LabelText(app.pin_type_label(n), "%d", n);
            }
            ImGui::Text("Node Type (drag)");

            for (auto n = 0u; n < app.node_type_count(); n++)
            {
                ImGui::Selectable(app.node_type_label(n));

                ImGuiDragDropFlags src_flags
                    = ImGuiDragDropFlags_SourceNoDisableHover // Keep the source displayed as
                                                              // hovered
                    | ImGuiDragDropFlags_SourceNoPreviewTooltip
                    | ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local,
                                                                   // we disable the feature of
                                                                   // opening foreign treenodes/tabs
                                                                   // while dragging
                // src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
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

        comm.join();

        return 0;
    }
};

int main(int argc, const char** argv)
{
    auto root = nil::cli::Node::root<nil::cli::nodes::Help>(std::cout);
    root.add<GUI>("gui", "renderer");
    root.add<EXT>("ext", "external");
    return root.run(argc, argv);
}
