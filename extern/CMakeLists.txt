cmake_minimum_required(VERSION 3.23)

if(ENRANGED_BUILD_TESTS)
  option(BUILD_GMOCK "" OFF)
  option(INSTALL_GTEST "" OFF)
  add_subdirectory(googletest)
endif()

if(ENRANGED_BUILD_BENCHMARKS)
  option(BENCHMARK_ENABLE_TESTING "")
  option(BENCHMARK_ENABLE_INSTALL "")
  add_subdirectory(benchmark)
endif()
