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
        publish/nil/gate/traits.hpp
        publish/nil/gate/Core.hpp
        publish/nil/gate/REdge.hpp
        publish/nil/gate/MEdge.hpp
        publish/nil/gate/IEdge.hpp
        publish/nil/gate/detail/edge/Data.hpp
        publish/nil/gate/detail/edge/Async.hpp
        publish/nil/gate/detail/edge/Batch.hpp
        publish/nil/gate/detail/edge/Internal.hpp
        publish/nil/gate/detail/Node.hpp
        publish/nil/gate/detail/Tasks.hpp
        publish/nil/gate/detail/INode.hpp
        publish/nil/gate/detail/callable_traits.hpp
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

add_test_subdirectory()
install_for_package(${PROJECT_NAME})
