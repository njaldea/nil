project(sandbox_clix)

find_package(nil COMPONENTS clix CONFIG REQUIRED)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::clix)