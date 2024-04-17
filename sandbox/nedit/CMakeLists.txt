project(sandbox_nedit)

add_subdirectory(messages/gen/nedit/messages)

find_package(nil COMPONENTS gate clix service CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(unofficial-imgui-node-editor CONFIG REQUIRED)

###
# glfw
# sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

add_library(${PROJECT_NAME}_lib ext/app/sort_nodes.cpp ext/app/sort_nodes.hpp)

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
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_proto)
target_link_libraries(${PROJECT_NAME} PRIVATE dev)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::clix)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::service)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::gate)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::imgui-node-editor::imgui-node-editor)
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_NO_DEPRECATED)

add_test_subdirectory()