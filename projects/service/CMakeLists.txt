project(service)

if(NOT ENABLE_FEATURE_SERVICE)
    return()
endif()

find_package(Boost REQUIRED)

set(
    HEADERS
        publish/nil/service.hpp
        publish/nil/service/codec.hpp
        publish/nil/service/TypedHandler.hpp
        publish/nil/service/IService.hpp
        publish/nil/service/udp/Client.hpp
        publish/nil/service/udp/Server.hpp
        publish/nil/service/tcp/Client.hpp
        publish/nil/service/tcp/Server.hpp
        publish/nil/service/ws/Client.hpp
        publish/nil/service/ws/Server.hpp
        src/tcp/Connection.hpp
        src/ws/Connection.hpp
        src/utils.hpp
)

set(
    SOURCES
        src/codec.cpp
        src/udp/Client.cpp
        src/udp/Server.cpp
        src/tcp/Client.cpp
        src/tcp/Server.cpp
        src/tcp/Connection.cpp
        src/ws/Client.cpp
        src/ws/Server.cpp
        src/ws/Connection.cpp
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(
    ${PROJECT_NAME}
    SYSTEM PRIVATE
    $<TARGET_PROPERTY:Boost::boost,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_NO_DEPRECATED)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_test_subdirectory()
install_for_package(${PROJECT_NAME})