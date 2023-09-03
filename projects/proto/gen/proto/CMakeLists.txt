project(proto)

include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    type.proto
    message.proto
    request.proto
)

add_library(${PROJECT_NAME} ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)