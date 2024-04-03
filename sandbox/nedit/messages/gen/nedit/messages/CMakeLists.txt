project(sandbox_nedit_proto)

find_package(Protobuf CONFIG REQUIRED)

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    type.proto
    state.proto
    node_state.proto
    metadata.proto
    control_update.proto
)

add_library(${PROJECT_NAME} ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf-lite)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "")

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../../..)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-conversion)
endif()