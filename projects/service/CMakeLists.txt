project(service)

set(
    HEADERS
        public/service/IService.hpp
        public/service/ws/Client.hpp
        public/service/ws/Server.hpp
)

set(
    SOURCES
        src/ws/Client.cpp
        src/ws/Server.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC public)

# TODO: figure out how to do it in a more modern way
# install(DIRECTORY include/ DESTINATION include)