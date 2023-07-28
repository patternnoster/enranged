cmake_minimum_required(VERSION 3.23)

option(ENRANGED_BUILD_BENCHMARKS "Build benchmarks for this library" OFF)
option(ENRANGED_BUILD_TESTS      "Build tests for this library"      OFF)

if(ENRANGED_BUILD_BENCHMARKS)
  set(CMAKE_BUILD_TYPE_INIT Release)
endif()

project(enranged LANGUAGES CXX)

include(GNUInstallDirs)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(enranged INTERFACE)
target_compile_features(enranged INTERFACE cxx_std_20)
target_include_directories(enranged INTERFACE
  $<BUILD_INTERFACE:${INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_subdirectory(extern)

if(ENRANGED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(ENRANGED_BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()

install(TARGETS enranged EXPORT enranged)
install(DIRECTORY ${INCLUDE_DIR}/enranged DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT enranged
  FILE enrangedConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/enranged)
