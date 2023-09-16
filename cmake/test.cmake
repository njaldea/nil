set(ENABLE_TEST     OFF CACHE BOOL "[0 | OFF - 1 | ON]: build tests?")

if(ENABLE_TEST)
    set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
    find_package(GTest CONFIG REQUIRED)
    include(CTest)
endif()

function(add_test_executable TARGET)
    add_executable(${TARGET} ${ARGN})
    add_test(NAME ${TARGET} COMMAND ${TARGET})
    add_dependencies(${TARGET} ${TARGET})
endfunction()

function(add_test_subdirectory)
    if(ENABLE_TEST)
        add_subdirectory(test)
    endif()
endfunction()