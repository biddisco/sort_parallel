########################################
# General examples
########################################

set(SRCS
  100_threads  default_threads  single_thread
)

foreach(example ${SRCS})
  add_executable(example_${example} example_${example}.cpp)
endforeach()



########################################
# HPX
########################################
if (SORT_HAS_HPX)

    set(CMAKE_CXX_FLAGS ${HPX_CXX_FLAGS})
    include_directories(
      ${HPX_INCLUDE_DIRS}
      ${PROJECT_SOURCE_DIR}/include
    )

    set(HPX_SRCS
      hpx_threads
    )
    
    foreach(example ${HPX_SRCS})
      add_executable(example_${example} example_${example}.cpp)
        hpx_setup_target(example_${example})
        target_link_libraries(example_${example} ${Boost_LIBRARIES})
    endforeach()
    
endif()