include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
  file_generator
  file_generator.cpp
)

#------------------------------------------------------------------------------
# includes and options for the benchmark
#------------------------------------------------------------------------------
set(BENCHMARK_LIBRARIES ${BENCHMARK_LIBRARIES} ${Boost_LIBRARIES})

if (SORT_HAS_TBB)
  include_directories(${TBB_INCLUDE_DIR})
  add_definitions(-DSORT_HAS_TBB)
  set(BENCHMARK_LIBRARIES ${BENCHMARK_LIBRARIES} ${TBB_LIBRARIES})
endif()

if (SORT_HAS_HPX)
  include_directories(${HPX_INCLUDE_DIRS})
  add_definitions(-DSORT_HAS_HPX)
  set(BENCHMARK_LIBRARIES ${BENCHMARK_LIBRARIES} ${TBB_LIBRARIES})
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-DSORT_HAS_GCC_PARALLEL)
  add_definitions(-D_GLIBCXX_USE_SCHED_YIELD)
endif()

#------------------------------------------------------------------------------
# Benchmark
#------------------------------------------------------------------------------
add_executable(
  benchmark
  benchmark.cpp
)
target_link_libraries(benchmark ${BENCHMARK_LIBRARIES})

if (SORT_HAS_HPX)
  hpx_setup_target(benchmark)
  target_link_libraries(benchmark)
endif()

#------------------------------------------------------------------------------
# restart
#------------------------------------------------------------------------------
add_executable(
  restart
  restart.cpp
)
target_link_libraries(restart ${BENCHMARK_LIBRARIES})

if (SORT_HAS_HPX)
  hpx_setup_target(restart)
  target_link_libraries(restart)
endif()
