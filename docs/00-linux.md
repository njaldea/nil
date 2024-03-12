# Linux setup

## My setup

- I use wsl ubuntu 22.04
- just install build-essential which will normally install gcc 11
- install latest cmake via snap (3.28.1)
    - if cmake configure step is slow, build it from source
    - `wget https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1.tar.gz`
- install clang-format/clang-tidy 18
    - `sudo add-apt-repository 'deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy main'`
    - `sudo apt install clang-format-18`
    - `sudo apt install clang-tidy-18`
    - `sudo apt install clangd-18`
- install via convenience script
    - `wget https://apt.llvm.org/llvm.sh`
    - `chmod +x llvm.sh`
    - `sudo ./llvm.sh 18 all`

## Coverage

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
