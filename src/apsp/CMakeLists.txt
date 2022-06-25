cmake_minimum_required(VERSION 3.14.0)
project(apsp VERSION 0.1.0)

# Include Google Test commands to simplify Google Tests integration
#
include(GoogleTest)

# Include a special Fetch Content command (to simplify usage of Google Tests)
#
include(FetchContent)

# Find OpenMP package
#
find_package(OpenMP)

# Declare Google Tests and Google Benchmark content
#
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.6.1
)

# Set main compilation options
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Initialise Google Tests library entry point
#
set(BUILD_GMOCK OFF)

# Initialise Google Benchmarks library options
#
# - Disable Google Tests
set(BENCHMARK_ENABLE_TESTING     OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)

# Make Google Tests and Google Benchmark available to the rest of the build
#
FetchContent_MakeAvailable(googletest googlebenchmark)

# Configure global includes for all configuration
# and targets
#
set(INC_LIST ../_common; src/utilz)

# Configure targets entry points
#
set(APP_SRC_LIST src/_application.cpp)
set(TST_SRC_LIST src/_test.cpp)
set(BNK_SRC_LIST src/_benchmark.cpp)

# Initialise include directories
#
include_directories(${INC_LIST})

# Enable testing
#
enable_testing()

# Initialise OpenMP compatible targets
#
list(APPEND omp_targets 00_omp)
list(APPEND omp_targets 01_omp)
list(APPEND omp_targets 03_omp)

# Initialise algorithms targets
#
list(APPEND targets_names 00)
list(APPEND targets_names 00_omp)
list(APPEND targets_names 01)
list(APPEND targets_names 01_omp)
list(APPEND targets_names 02)
list(APPEND targets_names 03)
list(APPEND targets_names 03_omp)

# Initialise algorithms aliases
#
list(APPEND targets_alias 00)
list(APPEND targets_alias 00)
list(APPEND targets_alias 01)
list(APPEND targets_alias 01)
list(APPEND targets_alias 02)
list(APPEND targets_alias 03)
list(APPEND targets_alias 03)

# Initialise length
#
list(LENGTH targets_names _length)
math(EXPR length "${_length}-1")

# Initialise targets
foreach(index RANGE ${length})
  list(GET targets_names ${index} t_name)
  list(GET targets_alias ${index} t_alias)

  add_executable(_application-v${t_name} ${APP_SRC_LIST})
  add_executable(_test-v${t_name}        ${TST_SRC_LIST})
  add_executable(_benchmark-v${t_name}   ${BNK_SRC_LIST})

  target_include_directories(_application-v${t_name} PRIVATE src/variants/${t_alias})
  target_include_directories(_test-v${t_name}        PRIVATE src/variants/${t_alias})
  target_include_directories(_benchmark-v${t_name}   PRIVATE src/variants/${t_alias})

  target_link_libraries(_test-v${t_name}      PRIVATE GTest::gtest_main)
  target_link_libraries(_benchmark-v${t_name} PRIVATE benchmark::benchmark_main)

  list(FIND omp_targets ${t_name} SUPPORTS_OPENMP)

  # If compiler supports OpenMP, our build variant is to use OpenMP and
  # algorithm supports OpenMP, then link OpenMP libraries and compiler options
  #
  if((OpenMP_CXX_FOUND) AND (SUPPORTS_OPENMP GREATER -1))
    target_link_libraries(_application-v${t_name} PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(_test-v${t_name} PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(_benchmark-v${t_name} PUBLIC OpenMP::OpenMP_CXX)
  endif()

  # Discover Google Tests in the target
  #
  gtest_discover_tests(_test-v${t_name})
endforeach()

set(CPACK_PROJECt_nameAME ${PROJECt_nameAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
