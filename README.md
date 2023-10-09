## The enranged library

A collection of various additions to the C++20 standard `<ranges>` library. Still work in progress. See [documentation](doc/index.md) for the list of features implemented so far.

### Installation

The library is header-only. To install the headers along with the CMake configuration file (for `find_package`) one can use the standard CMake procedure:
```sh
cmake $SOURCE_PATH
cmake --install . --prefix=$INSTALL_PATH
```

### Benchmarks

Tests were performed on Intel Core i7-1260P 12th Gen 4.7 GHz (L1: D48+I32 KiB, L2: 1280 KiB, L3: 18432 KiB) on a single core, on a list of 10'000'000 random 32-bit integers. The memory was initially shuffled to force cache misses. The application was compiled with the maximum optimization flags (-O3, /O2). For `bucket_sort`, 32 buckets with the corresponding bit shift equality as the equivalence relation was used. Additionally, a simple straightforward singly linked list implementation was tested as well for comparison. The latest versions of the standard library for the corresponding versions of compilers were used. The GCC and Clang tests were running on Linux (kernel 6.4.12, Arch) and the MSVC test on Windows 11.

![plot_ok](https://github.com/patternnoster/enranged/assets/98361268/aea0e816-f135-485c-8604-905317fdf121)

See the [benchmarks](benchmark/) directory for the source code.

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
