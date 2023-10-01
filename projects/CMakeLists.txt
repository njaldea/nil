add_subdirectory(cli)
add_subdirectory(service)
add_subdirectory(pulse)

# this is done in this folder to move the generated files
# to a more desirable include folder
add_subdirectory(proto/gen/proto)