# nil

Temporary project containing experimentation with c++. Back to the roots.

## To Configure

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

|            | feature name | flag                          | description                                   |
| ---------- | ------------ | ----------------------------- | --------------------------------------------- |
| cli        | cli          | `-DENABLE_FEATURE_CLI=ON`     | command line parser / boost::program_options  |
| service    | service      | `-DENABLE_FEATURE_SERVICE=ON` | event handling / boost::asio                  |
| proto      | proto        | `-DENABLE_FEATURE_PROTO=ON`   | testing of using protobuf                     |
| tests      | test         | `-DENABLE_TEST=ON`            | tests / might require other targets           |
| clang-tidy |              | `-DENABLE_CLANG_TIDY=ON`      | enable clang-tidy for all source files        |

to enable cli:

```
> cmake [source folder]                                                      \
>     -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake  \
>     -DVCPKG_MANIFEST_FEATURES="cli"                                        \
>     -DENABLE_FEATURE_CLI=ON
```

TODO: simplify this process.

---

Or just use ccmake to toggle the flags

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