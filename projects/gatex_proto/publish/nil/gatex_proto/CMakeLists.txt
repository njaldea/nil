project(gatex_proto)

find_package(Protobuf CONFIG REQUIRED)

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    identity.proto
    type.proto
    graph.proto
    metadata.proto
    node_state.proto
    control_update.proto
    imgui_metadata.proto
)

add_library(${PROJECT_NAME} ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf-lite)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "")

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../..)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-conversion)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-conversion)
endif()