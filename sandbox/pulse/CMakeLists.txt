project(sandbox_pulse)

if(NOT ENABLE_FEATURE_PULSE)
    return()
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE pulse)