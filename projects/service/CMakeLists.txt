project(service)

find_package(Boost REQUIRED)

if(NOT ENABLE_FEATURE_SERVICE)
    return()
endif()

set(
    HEADERS
        publish/nil/service.hpp
        publish/nil/service/IService.hpp
        publish/nil/service/tcp/Client.hpp
        publish/nil/service/tcp/Server.hpp
)

set(
    SOURCES
        src/tcp/Client.cpp
        src/tcp/Server.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)
target_include_directories(${PROJECT_NAME} PUBLIC publish)
