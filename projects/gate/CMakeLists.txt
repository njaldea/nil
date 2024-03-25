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
        publish/nil/gate/Batch.hpp
        publish/nil/gate/Core.hpp
        publish/nil/gate/IEdge.hpp
        publish/nil/gate/types.hpp
        publish/nil/gate/api/uniform.hpp
        publish/nil/gate/bias/compatibility.hpp
        publish/nil/gate/bias/edgify.hpp
        publish/nil/gate/detail/DataEdge.hpp
        publish/nil/gate/detail/ICallable.hpp
        publish/nil/gate/detail/INode.hpp
        publish/nil/gate/detail/Node.hpp
        publish/nil/gate/detail/Tasks.hpp
        publish/nil/gate/detail/traits/callable.hpp
        publish/nil/gate/detail/traits/node.hpp
        publish/nil/gate/detail/traits/types.hpp
        publish/nil/gate/detail/validation/edge.hpp
        publish/nil/gate/detail/validation/node.hpp
        publish/nil/gate/edges/Batch.hpp
        publish/nil/gate/edges/Compatible.hpp
        publish/nil/gate/edges/Mutable.hpp
        publish/nil/gate/edges/ReadOnly.hpp
        publish/nil/gate/nodes/Scoped.hpp
)

add_library(${PROJECT_NAME} INTERFACE ${HEADERS})

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_test_subdirectory()
install_for_package(${PROJECT_NAME})
