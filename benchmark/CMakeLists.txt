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

#------------------------------------------------------------------------------
# TBB
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(TBB)
if (TBB_FOUND)
  set(BOOST_SORT_WITH_TBB 1)
endif()

if (BOOST_SORT_WITH_TBB)
  include_directories(${TBB_INCLUDE_DIR})
  add_executable(
    benchmark_CLANG_TBB
    CLANG/src/benchmark_CLANG_TBB.cpp
  )
  target_link_libraries(benchmark_CLANG_TBB ${TBB_LIBRARIES})

  add_executable(
    benchmark_CLANG_TBB_BOOST
    CLANG/src/benchmark_CLANG_TBB_BOOST.cpp
  )
  target_link_libraries(benchmark_CLANG_TBB_BOOST ${TBB_LIBRARIES})
endif()

