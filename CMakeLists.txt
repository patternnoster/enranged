cmake_minimum_required(VERSION 3.23)

project(enranged LANGUAGES CXX)

option(ENRANGED_BUILD_TESTS "Build tests for this library" OFF)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(enranged INTERFACE)
target_compile_features(enranged INTERFACE cxx_std_20)
target_include_directories(enranged INTERFACE $<BUILD_INTERFACE:${INCLUDE_DIR}>)

if(ENRANGED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(extern)
  add_subdirectory(test)
endif()
