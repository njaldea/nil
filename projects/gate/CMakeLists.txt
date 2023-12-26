project(gate)

if(NOT ENABLE_FEATURE_GATE)
    return()
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
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# add_test_subdirectory()
install_for_package(${PROJECT_NAME})
