project(sandbox_nedit)

if(NOT ENABLE_FEATURE_NEDIT)
    return()
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(unofficial-imgui-node-editor CONFIG REQUIRED)

###
# glfw
# sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

set(
    SOURCES
        main.cpp
        app/App.cpp
        app/Link.cpp
        app/Node.cpp
        app/Pin.cpp
        app/ShadowNode.cpp
        app/FlowIcon.cpp
)

set(
    HEADERS
        app/App.hpp
        app/Link.hpp
        app/Node.hpp
        app/Pin.hpp
        app/ShadowNode.hpp
        app/FlowIcon.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE service)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::imgui-node-editor::imgui-node-editor)
