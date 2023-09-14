project(cli-test)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cli)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest_main)

add_test(
    NAME ${PROJECT_NAME}
    COMMAND ./${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)