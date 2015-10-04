cmake_minimum_required(VERSION 2.8)
project(sort_parallel)

# Setup cmake policies.
foreach(p
    CMP0012 CMP0013 CMP0014
    CMP0022 CMP0025 CMP0043
    CMP0053
  )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

#--------------------------------------------------
# Setup Output directories
#--------------------------------------------------
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
)
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
)

#------------------------------------------------------------------------------
# Boost
#------------------------------------------------------------------------------
set(Boost_ADDITIONAL_VERSIONS
    ${Boost_ADDITIONAL_VERSIONS}
    "1.59.0" "1.59"
    "1.58.0" "1.58"
    "1.57.0" "1.57")

find_package(Boost
  1.55
  REQUIRED
  COMPONENTS
    regex date_time serialization
    program_options filesystem log log_setup system thread
)

#------------------------------------------------------------------------------
# include dirs
#------------------------------------------------------------------------------
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
#  ${CMAKE_CURRENT_SOURCE_DIR}/include/boost/sort/parallel
)

#------------------------------------------------------------------------------
# tests
#------------------------------------------------------------------------------
add_subdirectory(test)
add_subdirectory(benchmark)

