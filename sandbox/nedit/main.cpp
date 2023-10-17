#include <imgui-node-editor/imgui_node_editor.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include "app/App.hpp"

#include <iostream>

int main()
{
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
        ImGui::BulletText("Drag and drop");
        static const char* names[] = {"0", "1", "2", "3", "4", "5"};

        for (auto n = 0u; n < 5u; n++)
        {
            ImGui::Selectable(names[n]);

            ImGuiDragDropFlags src_flags
                = ImGuiDragDropFlags_SourceNoDisableHover // Keep the source displayed as hovered
                | ImGuiDragDropFlags_SourceNoPreviewTooltip
                | ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we
                                                               // disable the feature of opening
                                                               // foreign treenodes/tabs while
                                                               // dragging
            // src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
            if (ImGui::BeginDragDropSource(src_flags))
            {
                app.try_create(n);
                ImGui::SetDragDropPayload("DND_DEMO_NAME", &n, sizeof(int));
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

    return 0;
}
