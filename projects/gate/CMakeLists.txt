project(gate)

if(NOT ENABLE_FEATURE_GATE)
    return()
endif()

if(NOT ENABLE_FEATURE_UTILS)
    message(FATAL_ERROR "gate library requires ENABLE_FEATURE_UTILS")
endif()

set(
    HEADERS
        publish/nil/gate.hpp
        publish/nil/gate/Builder.hpp
        publish/nil/gate/Core.hpp
        publish/nil/gate/REdge.hpp
        publish/nil/gate/MEdge.hpp
        publish/nil/gate/IEdge.hpp
        publish/nil/gate/detail/Edge.hpp
        publish/nil/gate/detail/Node.hpp
        publish/nil/gate/detail/INode.hpp
        publish/nil/gate/detail/types.hpp
        publish/nil/gate/detail/validation/edge.hpp
        publish/nil/gate/detail/validation/node.hpp
)

add_library(${PROJECT_NAME} INTERFACE ${HEADERS})

target_link_libraries(${PROJECT_NAME} INTERFACE utils)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# add_test_subdirectory()
install_for_package(${PROJECT_NAME})

# intellisense in vscode does not work out of the box for interface targets since there is no file to compile
# the target below is created to help intellisense.
# to load, either do one of the following:
#  -  open the cpp file
#  -  from header file, trigger switch to header/source
# ref: https://developercommunity.visualstudio.com/t/intellisense-not-working-properly-for-interface-he/1191108
add_library(${PROJECT_NAME}-intellisense STATIC src/detail/types.cpp)
target_link_libraries(${PROJECT_NAME}-intellisense PRIVATE ${PROJECT_NAME})