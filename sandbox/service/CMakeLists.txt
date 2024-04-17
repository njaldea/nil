project(sandbox_service)

find_package(nil COMPONENTS clix service CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::clix)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::service)
