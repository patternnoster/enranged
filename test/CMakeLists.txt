cmake_minimum_required(VERSION 3.23)

add_executable(enranged_tests
  limits_tests.cpp
  splicing_tests.cpp
)
target_link_libraries(enranged_tests PRIVATE enranged gtest_main)

# Show all warnings because we're pedantic (and also all and extra)
target_compile_options(enranged_tests PRIVATE
  $<IF:$<BOOL:${MSVC}>, /W3, -Wall -Wpedantic -Wextra>)

include(GoogleTest)
gtest_discover_tests(enranged_tests)
