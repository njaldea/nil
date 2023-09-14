# nil

Temporary project containing experimentation with c++. Back to the roots.

## To Configure

> cd [build folder]
> cmake [build folder] -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake

## To enable targets

| Target   | Flag                      |
| -------- | ------------------------- |
| cli      | `-DENABLE_FEATURE_CLI=ON` |

## To enable tests

> -DENABLE_TEST=ON

## To enable sandbox

> -DENABLE_SANDBOX=ON

---

Or just use ccmake to toggle the flags

## when trying local build for vcpkg

when doing local trial:
- cd to external directory (to disable manifest mode)
- `vcpkg install nil --overlay-ports=<path_to_repo>/ports/nil`
- `vcpkg remove nil --overlay-ports=<path_to_repo>/ports/nil`
- `git rev-parse HEAD:ports/nil`
    - update versions git-tree