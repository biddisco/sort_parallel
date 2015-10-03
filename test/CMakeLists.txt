set(SRCS
  algorithm
  buffer
  heap_sort
  insertion_sort
  intro_sort
  parallel_sort
  parallel_stable_sort
  range
  sample_sort
  smart_merge_sort
  spinlock
  stack_cnc
)

foreach(test ${SRCS})
  add_executable(test_${test} test_${test}.cpp)
  add_test(
    NAME test_${test}
    COMMAND test_${test}
  )
endforeach()


