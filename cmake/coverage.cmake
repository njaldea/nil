set(ENABLE_COVERAGE OFF CACHE BOOL "[0 | OFF - 1 | ON]: build with coverage?")

if(NOT ENABLE_COVERAGE)
    return()
endif()

add_compile_options(-fprofile-instr-generate)
add_compile_options(-fcoverage-mapping)
add_link_options(-fprofile-instr-generate)
add_link_options(-fcoverage-mapping)