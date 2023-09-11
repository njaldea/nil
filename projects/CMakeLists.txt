if(NOT ENABLE_FEATURE_CLI AND NOT ENABLE_FEATURE_SERVICE AND NOT ENABLE_FEATURE_PROTO)
    message(FATAL_ERROR "No feature is enabled")
endif()

if(ENABLE_FEATURE_CLI)
    add_subdirectory(cli)
endif()

if(ENABLE_FEATURE_SERVICE)
    add_subdirectory(service)
endif()

if(ENABLE_FEATURE_PROTO)
    # this is done in this folder to move the generated files
    # to a more desirable include folder
    add_subdirectory(proto/gen/proto)
endif()