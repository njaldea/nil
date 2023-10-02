# nil

Temporary project containing experimentation with c++. Back to the roots.

## To Configure

### manual

```
> cd ${BUILD_FOLDER}
> cmake [SOURCE_FOLDER]                                                      \
>     -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake  \
>     -DVCPKG_MANIFEST_FEATURES="cli;test;sandbox"
>     -DENABLE_FEATURE_CLI=ON`
```

features are necessary for vcpkg to pull and install all dependencies   <br/>
targets are necessary to actually build the source code                 <br/>
make sure to enable a feature when working on a specific target

|                                          | feature name | flag                          | description                                           |
| ---------------------------------------- | ------------ | ----------------------------- | ----------------------------------------------------- |
| [cli](projects/cli/README.md)            | cli          | `-DENABLE_FEATURE_CLI=ON`     | command line parser / boost::program_options          |
| [service](projects/service/README.md)    | service      | `-DENABLE_FEATURE_SERVICE=ON` | event handling / boost::asio                          |
| [pulse](projects/pulse/README.md)        | pulse        | `-DENABLE_FEATURE_PULSE=ON`   | straight forward implementation of reactive variables |
| proto                                    | proto        | `-DENABLE_FEATURE_PROTO=ON`   | testing of using protobuf                             |
| tests                                    | test         | `-DENABLE_TEST=ON`            | tests / might require other targets                   |
| clang-tidy                               |              | `-DENABLE_CLANG_TIDY=ON`      | enable clang-tidy for all source files                |

to enable cli:

```
> cmake [source folder]                                                      \
>     -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake  \
>     -DVCPKG_MANIFEST_FEATURES="cli"                                        \
>     -DENABLE_FEATURE_CLI=ON
```

### use cmake presets

- export `VCPKG_ROOT` pointing to where vcpkg is located
- `cmake . --preset dev`

### rely on vscode

- make sure that `VCPKG_ROOT` is defined and visible to vscode
- if not, override `$env{VCPKG_ROOT}` in cmake presets
- or manually configure cmake (from cmake presets) first

## To publish a new version

- edit [portfile.cmake](ports/nil/portfile.cmake)
    - `REF` == current commit hash
    - `SHA512` == acquire via steps in next section
- commit changes
- run `git rev-parse HEAD:ports/nil`
- update/add git-tree in [versions/n-/nil.json](versions/n-/nil.json)
- commit changes
- push to remote

### To acquire SHA512

- cd to external directory to disable vcpkg manifest mode
- remove install nil in vcpkg
    - `vcpkg remove nil --overlay-ports=<path_to_repo>/ports/nil`
- install local version to vcpkg
    - `vcpkg install nil --overlay-ports=<path_to_repo>/ports/nil`
- from the error message, copy the hash printed

### Coverage

requires the following:
- lcov
- genthml
- npx

requires enabling the following flags:
- ENABLE_TEST
- ENABLE_COVERAGE

targets:
- make clean
- make -j
- make coverage_init
- ctest
- make coverage_generate
- make coverage_serve

### Consuming header only libraries

Depending on vcpkg and simply using `target_link_library` should work with a drawback where in if the target is published, it will be required for users to have them as well.
If the target is linked to produce static/shared libraries, the headers are not really necessary for users to know.

To prevent leakage, instead of `target_link_library`, the following can be used instead:

```
target_include_directories(
    ${PROJECT_NAME}
    SYSTEM PRIVATE
    $<TARGET_PROPERTY:${HEADER_LIBRARY_HERE},INTERFACE_INCLUDE_DIRECTORIES>
)
```