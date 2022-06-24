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
list(APPEND omp_targets 00)
list(APPEND omp_targets 01)
list(APPEND omp_targets 03)

# Initialise targets
foreach(t 00 01 02 03)
  add_executable(_application-v${t} ${APP_SRC_LIST})
  add_executable(_test-v${t}        ${TST_SRC_LIST})
  add_executable(_benchmark-v${t}   ${BNK_SRC_LIST})

  target_include_directories(_application-v${t} PRIVATE src/variants/${t})
  target_include_directories(_test-v${t}        PRIVATE src/variants/${t})
  target_include_directories(_benchmark-v${t}   PRIVATE src/variants/${t})

  target_link_libraries(_test-v${t}      PRIVATE GTest::gtest_main)
  target_link_libraries(_benchmark-v${t} PRIVATE benchmark::benchmark_main)

  list(FIND omp_targets ${t} SUPPORTS_OPENMP)

  if((OpenMP_CXX_FOUND) AND (ENABLE_OPENMP) AND (SUPPORTS_OPENMP GREATER -1))
    target_link_libraries(_application-v${t} PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(_test-v${t} PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(_benchmark-v${t} PUBLIC OpenMP::OpenMP_CXX)
  endif()

  # Discover Google Tests in the target
  #
  gtest_discover_tests(_test-v${t})
endforeach()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
