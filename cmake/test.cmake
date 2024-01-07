set(ENABLE_TEST     OFF CACHE BOOL "[0 | OFF - 1 | ON]: build tests?")

if(ENABLE_TEST)
    set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
    find_package(GTest CONFIG REQUIRED)
    include(CTest)
endif()

function(add_test_executable TARGET)
    add_executable(${TARGET} ${ARGN})
    if(ENABLE_CLANG_TIDY AND CMAKE_CXX_CLANG_TIDY)
        set_target_properties(${TARGET} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};-checks=-readability-function-cognitive-complexity")
    endif()

    add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction()

function(add_test_subdirectory)
    if(ENABLE_TEST)
        add_subdirectory(test)
    endif()
endfunction()