include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
  file_generator
  CLANG/src/file_generator.cpp
)

add_executable(
  benchmark_CLANG
  CLANG/src/benchmark_CLANG.cpp
)

add_executable(
  benchmark_CLANG_BOOST
  CLANG/src/benchmark_CLANG_BOOST.cpp
)

