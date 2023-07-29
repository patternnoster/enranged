## The enranged library

A collection of various additions to the C++20 standard `<ranges>` library. Still work in progress. See [documentation](doc/index.md) for the list of features implemented so far.

### Installation

The library is header-only. To install the headers along with the CMake configuration file (for `find_package`) one can use the standard CMake procedure:
```sh
cmake $SOURCE_PATH
cmake --install . --prefix=$INSTALL_PATH
```

### Running tests and benchmarks

Use the `ENRANGED_BUILD_TESTS=ON` CMake option to build the library tests on your system (requires the googletest submodule). One can run the following commands (in the source directory) to build and run the tests:
```sh
mkdir build && cd build
cmake .. -DENRANGED_BUILD_TESTS=ON && cmake --build .
ctest
```

To build the library benchmarks on your system, use the `ENRANGED_BUILD_BENCHMARKS=ON` CMake option (requires the google benchmark submodule). That will automatically change the build type to Release and enable optimizations:
```sh
mkdir build && cd build
cmake .. -DENRANGED_BUILD_BENCHMARKS=ON && cmake --build .
./benchmark/*_benchmarks
```
