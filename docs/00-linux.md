# Linux setup

## My setup

- I use wsl ubuntu 22.04
- install build-essential which will normally install gcc 11
- install clang-format/clang-tidy 18
    - this is mainly for the toolings
        - clang-format-18
        - clang-tidy-18
        - clangd-18
    - convenience script
        - `wget https://apt.llvm.org/llvm.sh`
        - `chmod +x llvm.sh`
        - `sudo ./llvm.sh 18 all`
    - manual
        - `sudo add-apt-repository 'deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy main'`
        - `sudo apt install clang-format-18`
        - `sudo apt install clang-tidy-18`
        - `sudo apt install clangd-18`
- install ninja
    - `sudo apt install ninja-build`
- install latest cmake via snap (3.28.1)
    - if cmake configure step is slow, build it from source
    - `wget https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1.tar.gz`
- setup vcpkg
    - set VCPKG_ROOT environment variable

## Coverage (will need to revisit due to clang usage)

requires the following:
- lcov
- genthml
- npx

requires enabling the following flags:
- ENABLE_TEST
- ENABLE_COVERAGE

targets:
- ninja clean
- ninja
- ninja coverage_init
- ctest
- ninja coverage_generate
- ninja coverage_serve
