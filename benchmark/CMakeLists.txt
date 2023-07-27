cmake_minimum_required(VERSION 3.23)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE MATCHES Release)
  message(WARNING "Building benchmarks in \"${CMAKE_BUILD_TYPE}\" (instead of Release)")
endif()

add_executable(sorting_benchmarks sorting_benchmarks.cpp)
target_link_libraries(sorting_benchmarks PRIVATE enranged benchmark_main)
target_include_directories(sorting_benchmarks PRIVATE ${CMAKE_SOURCE_DIR}/test)
