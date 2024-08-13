project(sandbox_wix)

find_package(nil COMPONENTS clix service CONFIG REQUIRED)

add_subdirectory(messages/gen/wix/messages)

add_executable(${PROJECT_NAME} main.cpp serve.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sandbox_wix_proto)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::clix)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::service)
