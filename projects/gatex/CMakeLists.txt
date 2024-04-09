project(gatex)

if(NOT ENABLE_FEATURE_CLI OR NOT ENABLE_FEATURE_SERVICE OR NOT ENABLE_FEATURE_GATE)
    return()
endif()

set(
    SOURCES
        Core.cpp
        Info.cpp
        serve.cpp
        parse.cpp
        load.cpp
)

set(
    HEADERS
        publish/nil/gatex.hpp
        publish/nil/gatex/Core.hpp
        publish/nil/gatex/Info.hpp
        publish/nil/gatex/identity.hpp
        publish/nil/gatex/INode.hpp
        publish/nil/gatex/IType.hpp
        publish/nil/gatex/RelaxedEdge.hpp
        publish/nil/gatex/serve.hpp
        publish/nil/gatex/parse.hpp
        publish/nil/gatex/load.hpp
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE sandbox_nedit_proto)
target_link_libraries(${PROJECT_NAME} PUBLIC gate)
target_link_libraries(${PROJECT_NAME} PUBLIC service)
target_link_libraries(${PROJECT_NAME} PUBLIC gatex_proto)
target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_ASIO_NO_DEPRECATED)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
