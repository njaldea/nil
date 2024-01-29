project(sandbox_nedit)

if(NOT ENABLE_FEATURE_CLI OR NOT ENABLE_FEATURE_SERVICE OR NOT ENABLE_FEATURE_GATE)
    return()
endif()

add_subdirectory(messages/gen/nedit/messages)

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(unofficial-imgui-node-editor CONFIG REQUIRED)

###
# glfw
# sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

add_library(${PROJECT_NAME}_lib STATIC ext/app/sort_nodes.cpp ext/app/sort_nodes.hpp)

set(
    SOURCES
        main.cpp
        ext/ext.cpp
        ext/app/install.cpp
        ext/app/Control.cpp
        gui/gui.cpp
        gui/app/App.cpp
        gui/app/Link.cpp
        gui/app/Node.cpp
        gui/app/Pin.cpp
        gui/app/FlowIcon.cpp
        gui/app/Control.cpp
        gui/app/IDs.cpp
)

set(
    HEADERS
        codec.hpp
        ext/ext.hpp
        ext/app/install.hpp
        ext/app/App.hpp
        ext/app/Control.hpp
        gui/gui.hpp
        gui/app/App.hpp
        gui/app/Link.hpp
        gui/app/Node.hpp
        gui/app/Pin.hpp
        gui/app/FlowIcon.hpp
        gui/app/Control.hpp
        gui/app/IDs.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/messages)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_proto)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE service)
target_link_libraries(${PROJECT_NAME} PRIVATE gate)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::imgui-node-editor::imgui-node-editor)

add_test_subdirectory()