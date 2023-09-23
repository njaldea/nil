project(service)

find_package(Boost REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

if(NOT ENABLE_FEATURE_SERVICE)
    return()
endif()

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    src/msg.proto
)

set(
    HEADERS
        publish/nil/service.hpp
        publish/nil/service/IService.hpp
        publish/nil/service/IHandler.hpp
        publish/nil/service/tcp/Client.hpp
        publish/nil/service/tcp/Server.hpp
        src/tcp/Connection.hpp
        ${PROTO_HEADER}
)

set(
    SOURCES
        src/tcp/Client.cpp
        src/tcp/Server.cpp
        src/tcp/Connection.cpp
        ${PROTO_SRC}
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotobuf)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install_for_package(${PROJECT_NAME})