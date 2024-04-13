# To Configure

## manual

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
| [pulse](projects/pulse/README.md)        | pulse        | `-DENABLE_FEATURE_PULSE=ON`   | straight forward implementation of reactive variables |
| [utils](projects/utils/README.md)        | utils        | `-DENABLE_FEATURE_UTILS=ON`   | collection of common utility libraries                |
| [service](projects/service/README.md)    | service      | `-DENABLE_FEATURE_SERVICE=ON` | event handling / boost::asio                          |
| [gate](projects/gate/README.md)          | gate         | `-DENABLE_FEATURE_GATE=ON`    | nil-/gate js to c++                                   |
| sandbox                                  | sandbox      | `-DENABLE_SANDBOX=ON`         | enable all features and sandbox                       |
| tests                                    | test         | `-DENABLE_TEST=ON`            | tests / might require other targets                   |
| clang-tidy                               |              | `-DENABLE_CLANG_TIDY=ON`      | enable clang-tidy for all source files                |

to enable cli:

```
> cmake [source folder]                                                      \
>     -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake  \
>     -DVCPKG_MANIFEST_FEATURES="cli"                                        \
>     -DENABLE_FEATURE_CLI=ON
```

## use configure scripts

- if you want to use gcc

```bash
> cd <repo>
> ./configure/gcc
```

- if you want to use clang

```bash
> cd <repo>
> ./configure/clang
```

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

### If intellisense does not work...

Intellisense (from vscode with its c++ and cmake extensions) has difficulty in resolving some symbols.
One example is for headers that are part of an INTERFACE library since they don't have source files.
With this, a dedicated source file is created to help.

```
add_library(${PROJECT_NAME}-intellisense <source file including the header files>)
target_link_libraries(${PROJECT_NAME}-intellisense PRIVATE ${PROJECT_NAME})
```

Additionally, switching header/source (Alt+O) would be a required step.