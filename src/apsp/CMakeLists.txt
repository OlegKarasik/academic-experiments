cmake_minimum_required(VERSION 3.20.0)

# Setup policies
#
cmake_policy(SET CMP0077 NEW)

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

# Find Intel ITT if needed (Windows)
#
if(WIN32)
  set(CMAKE_VTUNE_SDK_DIR "C:\\Program Files (x86)\\Intel\\oneAPI\\vtune\\latest\\sdk")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(itt_arch "64")
  else()
    set(itt_arch "32")
  endif()

  find_path(ITT_INCLUDE_DIRS ittnotify.h
    PATHS ${CMAKE_VTUNE_SDK_DIR}
    PATH_SUFFIXES include)

  find_path(ITT_LIBRARY_DIRS libittnotify.lib
    PATHS ${CMAKE_VTUNE_SDK_DIR}
    PATH_SUFFIXES lib${itt_arch})

  if(NOT ITT_INCLUDE_DIRS MATCHES NOTFOUND AND
     NOT ITT_LIBRARY_DIRS MATCHES NOTFOUND)

    message(STATUS "Found Intel ITT includes in ${ITT_INCLUDE_DIRS}")
    message(STATUS "Found Intel ITT libraries in ${ITT_LIBRARY_DIRS}")

    include_directories(${ITT_INCLUDE_DIRS})
    link_directories(${ITT_LIBRARY_DIRS})

    set(ITT_LIBRARY libittnotify)
    set(ITT_FOUND TRUE)

    message(STATUS "Intel ITT - FOUND")
  endif()
endif()

if (NOT ITT_FOUND)
  message(STATUS "Intel ITT - NOT FOUND")
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Intel") OR (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM"))
  set(ICL_ICX_FOUND TRUE)

  message(STATUS "Detected Intel ICL/ICX... Disabling google test and goole benchmarks...")
  message(STATUS "Intel ICL/ICX - FOUND")
endif()

if (NOT ICL_ICX_FOUND)
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
endif()

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
list(APPEND omp_targets "00-omp")
list(APPEND omp_targets "01-omp")
list(APPEND omp_targets "03-omp")

# Initialise algorithms targets
#
list(APPEND targets_names "00")
list(APPEND targets_names "00-omp")
list(APPEND targets_names "01")
list(APPEND targets_names "01-omp")
list(APPEND targets_names "02")
list(APPEND targets_names "03")
list(APPEND targets_names "03-omp")

# Initialise algorithms aliases
#
list(APPEND targets_alias "00")
list(APPEND targets_alias "00")
list(APPEND targets_alias "01")
list(APPEND targets_alias "01")
list(APPEND targets_alias "02")
list(APPEND targets_alias "03")
list(APPEND targets_alias "03")

# Initialise ITT targets if ITT is enabled
#
if (ITT_FOUND)
  list(APPEND itt_targets "00-itt")
  list(APPEND itt_targets "00-omp-itt")
  list(APPEND itt_targets "01-itt")
  list(APPEND itt_targets "01-omp-itt")
  list(APPEND itt_targets "02-itt")
  list(APPEND itt_targets "03-itt")
  list(APPEND itt_targets "03-omp-itt")

  list(APPEND omp_targets "00-omp-itt")
  list(APPEND omp_targets "01-omp-itt")
  list(APPEND omp_targets "03-omp-itt")

  list(APPEND targets_names "00-itt")
  list(APPEND targets_names "00-omp-itt")
  list(APPEND targets_names "01-itt")
  list(APPEND targets_names "01-omp-itt")
  list(APPEND targets_names "02-itt")
  list(APPEND targets_names "03-itt")
  list(APPEND targets_names "03-omp-itt")

  list(APPEND targets_alias "00")
  list(APPEND targets_alias "00")
  list(APPEND targets_alias "01")
  list(APPEND targets_alias "01")
  list(APPEND targets_alias "02")
  list(APPEND targets_alias "03")
  list(APPEND targets_alias "03")
endif()

# Initialise length
#
list(LENGTH targets_names _length)
math(EXPR length "${_length}-1")

# Initialise targets
foreach(index RANGE ${length})
  list(GET targets_names ${index} t_name)
  list(GET targets_alias ${index} t_alias)

  if (NOT ICL_ICX_FOUND)
    string(REPLACE - _ t_fixture ${t_name})
  endif()

  add_executable(_application-v${t_name} ${APP_SRC_LIST})

  target_include_directories(_application-v${t_name} PRIVATE src/variants/${t_alias})

  if (NOT ICL_ICX_FOUND)
    add_executable(_test-v${t_name}        ${TST_SRC_LIST})
    add_executable(_benchmark-v${t_name}   ${BNK_SRC_LIST})

    target_include_directories(_test-v${t_name}        PRIVATE src/variants/${t_alias})
    target_include_directories(_benchmark-v${t_name}   PRIVATE src/variants/${t_alias})

    target_link_libraries(_test-v${t_name}      PRIVATE GTest::gtest_main)
    target_link_libraries(_benchmark-v${t_name} PRIVATE benchmark::benchmark_main)

    target_compile_definitions(_test-v${t_name} PRIVATE FIXTURE_NAME=${t_fixture})
  endif()


  # If compiler supports OpenMP, our build variant is to use OpenMP and
  # algorithm supports OpenMP, then link OpenMP libraries and compiler options
  #
  if((OpenMP_CXX_FOUND) AND (${t_name} IN_LIST omp_targets))
    target_link_libraries(_application-v${t_name} PUBLIC OpenMP::OpenMP_CXX)

    if (NOT ICL_ICX_FOUND)
      target_link_libraries(_test-v${t_name} PUBLIC OpenMP::OpenMP_CXX)
      target_link_libraries(_benchmark-v${t_name} PUBLIC OpenMP::OpenMP_CXX)

      target_compile_definitions(_test-v${t_name} PRIVATE FIXTURE_NAME=${t_fixture})
    endif()
  endif()

  # If target is dedicated to ITT
  #
  if ((ITT_FOUND) AND (${t_name} IN_LIST itt_targets))
    target_compile_definitions(_application-v${t_name} PRIVATE ITT_TRACE)

    target_link_libraries(_application-v${t_name} PUBLIC ${ITT_LIBRARY})

    if (NOT ICL_ICX_FOUND)
      target_compile_definitions(_test-v${t_name} PRIVATE ITT_TRACE)
      target_compile_definitions(_benchmark-v${t_name} PRIVATE ITT_TRACE)

      target_link_libraries(_test-v${t_name} PUBLIC ${ITT_LIBRARY})
      target_link_libraries(_benchmark-v${t_name} PUBLIC ${ITT_LIBRARY})

      target_compile_definitions(_test-v${t_name} PRIVATE FIXTURE_NAME=${t_fixture})
    endif()
  endif()

  if (NOT ICL_ICX_FOUND)
    # Discover Google Tests in the target
    #
    gtest_discover_tests(_test-v${t_name})
  endif()
endforeach()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
