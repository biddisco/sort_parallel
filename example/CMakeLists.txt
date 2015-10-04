set(SRCS
  100_threads  default_threads  single_thread
)

foreach(example ${SRCS})
  add_executable(example_${example} example_${example}.cpp)
endforeach()


