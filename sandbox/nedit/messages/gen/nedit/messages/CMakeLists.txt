project(sandbox_nedit_proto)

find_package(Protobuf CONFIG REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    type.proto
    graph_update.proto
    node_info.proto
    pin_info.proto
)

add_library(${PROJECT_NAME} ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf-lite)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-conversion)
endif()