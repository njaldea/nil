if(NOT ENABLE_FEATURE_CLI AND NOT ENABLE_FEATURE_SERVICE AND NOT ENABLE_FEATURE_PROTO)
    message(FATAL_ERROR "No feature is enabled")
endif()

add_subdirectory(cli)
add_subdirectory(service)
add_subdirectory(pulse)

# this is done in this folder to move the generated files
# to a more desirable include folder
add_subdirectory(proto/gen/proto)