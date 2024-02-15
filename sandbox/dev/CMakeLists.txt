project(dev)

add_library(${PROJECT_NAME} nil/dev.cpp nil/dev.hpp)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})