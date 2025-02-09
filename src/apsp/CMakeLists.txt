cmake_minimum_required(VERSION 3.24.0)

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

# Find AK-Kernel if needed (Windows)
#
if (WIN32)
  cmake_host_system_information(
    RESULT CMAKE_KERNEL_ROOT_PATH
    QUERY WINDOWS_REGISTRY "HKLM\\SOFTWARE\\Oleg Karasik\\AK-Kernel\\1.30.0"
    VALUE "InstallPath")

  find_path(KERNEL_INCLUDE_DIRS Kernel{Core}.h
    PATHS ${CMAKE_KERNEL_ROOT_PATH}
    PATH_SUFFIXES include)

  find_path(KERNEL_LIBRARY_DIRS Kernel.Core.lib
    PATHS ${CMAKE_KERNEL_ROOT_PATH}
    PATH_SUFFIXES lib)

  if(NOT KERNEL_INCLUDE_DIRS MATCHES NOTFOUND AND
     NOT KERNEL_LIBRARY_DIRS MATCHES NOTFOUND)

    message(STATUS "Found AK-Kernel includes in ${KERNEL_INCLUDE_DIRS}")
    message(STATUS "Found AK-Kernel libraries in ${KERNEL_LIBRARY_DIRS}")

    include_directories(${KERNEL_INCLUDE_DIRS})
    link_directories(${KERNEL_LIBRARY_DIRS})

    set(KERNEL_LIBRARY Kernel.Core.lib)
    set(KERNEL_DLL "${CMAKE_KERNEL_ROOT_PATH}Kernel.Core.dll")
    set(KERNEL_FOUND TRUE)

    message(STATUS "AK-Kernel - FOUND")
  endif()
endif()

if (NOT KERNEL_FOUND)
  message(STATUS "AK-Kernel - NOT FOUND")
endif()

# Find Visual Studio if needed (Windows)
#
if (WIN32)
  # Thanks to markdewit_ies
  # for sharing this amazing way to find Visual Studio
  # https://discourse.cmake.org/t/visual-studio-install-path-and-directory-query/3684/8
  #
  find_program(VSWHERE_TOOL_PATH
    NAMES vswhere
    PATHS "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer")

  if (NOT ${vswhere})
    message(FATAL_ERROR "Could not locate vswhere.exe - unable to source vc redistributable")
  endif()

  execute_process(
    COMMAND "${VSWHERE_TOOL_PATH}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    OUTPUT_VARIABLE VS_INSTALL_PATH
    RESULT_VARIABLE VSWHERE_EXIT_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(VS_INSTALL_PATH)
    message(STATUS "Found Visual Studio includes in ${KERNEVS_INSTALL_PATHL_INCLUDE_DIRS}")
    message(STATUS "Found AK-Kernel libraries in ${KERNEL_LIBRARY_DIRS}")

    include_directories("${VS_INSTALL_PATH}\\VC\\Auxiliary\\VS\\include")

    set(VS_FOUND TRUE)

    message(STATUS "Visual Studio - FOUND")
  endif()
endif()

if (NOT VS_FOUND)
  message(STATUS "Visual Studio - NOT FOUND")
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Intel") OR (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM"))
  set(ICL_ICX_FOUND TRUE)

  message(STATUS "Detected Intel ICL/ICX... Disabling google test and goole benchmarks...")
  message(STATUS "Intel ICL/ICX - FOUND")
endif()

if (ICL_ICX_FOUND)
  set(SKIP_GSUITE TRUE)
  message(STATUS "Skipping compilation of tests and benchmarks because they aren't supported")
endif()

# Set main compilation options
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT SKIP_GSUITE)
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

# Initialise Kernel compatible targets (Windows only)
#
list(APPEND kernel_targets "05-kernel")

# Initialise OpenMP compatible targets
#
list(APPEND omp_targets "00-omp")
list(APPEND omp_targets "01-omp")
list(APPEND omp_targets "02-omp")
list(APPEND omp_targets "03-omp")
list(APPEND omp_targets "04-omp")

# Initialise algorithms targets
#
list(APPEND targets_names "00")
list(APPEND targets_names "00-omp")
list(APPEND targets_names "01")
list(APPEND targets_names "01-omp")
list(APPEND targets_names "02")
list(APPEND targets_names "02-omp")
list(APPEND targets_names "03")
list(APPEND targets_names "03-omp")
list(APPEND targets_names "04")
list(APPEND targets_names "04-omp")
list(APPEND targets_names "05-kernel")

# Initialise algorithms aliases
#
list(APPEND targets_alias "00")
list(APPEND targets_alias "00")
list(APPEND targets_alias "01")
list(APPEND targets_alias "01")
list(APPEND targets_alias "02")
list(APPEND targets_alias "02")
list(APPEND targets_alias "03")
list(APPEND targets_alias "03")
list(APPEND targets_alias "04")
list(APPEND targets_alias "04")
list(APPEND targets_alias "05")

# Initialise ITT targets if ITT is enabled
#
if (ITT_FOUND)
  list(APPEND itt_targets "00-itt")
  list(APPEND itt_targets "00-omp-itt")
  list(APPEND itt_targets "01-itt")
  list(APPEND itt_targets "01-omp-itt")
  list(APPEND itt_targets "02-itt")
  list(APPEND itt_targets "02-omp-itt")
  list(APPEND itt_targets "03-itt")
  list(APPEND itt_targets "03-omp-itt")
  list(APPEND itt_targets "04-itt")
  list(APPEND itt_targets "04-omp-itt")
  list(APPEND itt_targets "05-kernel-itt")

  list(APPEND omp_targets "00-omp-itt")
  list(APPEND omp_targets "01-omp-itt")
  list(APPEND omp_targets "02-omp-itt")
  list(APPEND omp_targets "03-omp-itt")
  list(APPEND omp_targets "04-omp-itt")

  list(APPEND targets_names "00-itt")
  list(APPEND targets_names "00-omp-itt")
  list(APPEND targets_names "01-itt")
  list(APPEND targets_names "01-omp-itt")
  list(APPEND targets_names "02-itt")
  list(APPEND targets_names "02-omp-itt")
  list(APPEND targets_names "03-itt")
  list(APPEND targets_names "03-omp-itt")
  list(APPEND targets_names "04-itt")
  list(APPEND targets_names "04-omp-itt")
  list(APPEND targets_names "05-kernel-itt")

  list(APPEND targets_alias "00")
  list(APPEND targets_alias "00")
  list(APPEND targets_alias "01")
  list(APPEND targets_alias "01")
  list(APPEND targets_alias "02")
  list(APPEND targets_alias "02")
  list(APPEND targets_alias "03")
  list(APPEND targets_alias "03")
  list(APPEND targets_alias "04")
  list(APPEND targets_alias "04")
  list(APPEND targets_alias "05-kernel-itt")
endif()

add_custom_target(tests)

# Initialise length
#
list(LENGTH targets_names _length)
math(EXPR length "${_length}-1")

# Initialise targets
foreach(index RANGE ${length})
  list(GET targets_names ${index} t_name)
  list(GET targets_alias ${index} t_alias)

  if (NOT SKIP_GSUITE)
    string(REPLACE - _ t_fixture ${t_name})
  endif()

  # If algorithm requires Kernel and Kernel isn't found then
  # skip the target
  #
  if((${t_name} IN_LIST kernel_targets) AND (NOT (KERNEL_FOUND) OR NOT (VS_FOUND)))
    message(STATUS "Skipping _application-v${t_name} because Kernel or Visual Studio is not found")
    continue()
  endif()

  # If algorithm requires OpenMP and OpenMP isn't found then
  # skip the target
  #
  if((${t_name} IN_LIST omp_targets) AND NOT (OpenMP_CXX_FOUND))
    message(STATUS "Skipping _application-v${t_name} because OpenMP is not found")
    continue()
  endif()

  add_executable(_application-v${t_name} ${APP_SRC_LIST})

  target_include_directories(_application-v${t_name} PRIVATE src/variants/${t_alias})

  if (NOT SKIP_GSUITE)
    add_executable(_test-v${t_name}        ${TST_SRC_LIST})
    add_executable(_benchmark-v${t_name}   ${BNK_SRC_LIST})

    add_dependencies(tests _test-v${t_name})

    target_include_directories(_test-v${t_name}        PRIVATE src/variants/${t_alias})
    target_include_directories(_benchmark-v${t_name}   PRIVATE src/variants/${t_alias})

    target_link_libraries(_test-v${t_name}      PRIVATE GTest::gtest_main)
    target_link_libraries(_benchmark-v${t_name} PRIVATE benchmark::benchmark_main)

    target_compile_definitions(_test-v${t_name} PRIVATE FIXTURE_NAME=${t_fixture})

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(_benchmark-v${t_name} PRIVATE "-fopt-info-vec-optimized" "-fopt-info-loop-optimized" "-save-temps" "-fverbose-asm")
    endif()
  endif()

  # If Kernel is found and target requires OpenMP,
  # then link Kernel libraries
  #
  if ((KERNEL_FOUND) AND (${t_name} IN_LIST kernel_targets))
    target_link_libraries(_application-v${t_name} PUBLIC ${KERNEL_LIBRARY})

    if (NOT SKIP_GSUITE)
      target_link_libraries(_test-v${t_name} PUBLIC ${KERNEL_LIBRARY})
      target_link_libraries(_benchmark-v${t_name} PUBLIC ${KERNEL_LIBRARY})
    endif()

    file(COPY ${KERNEL_DLL} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  # If compiler supports OpenMP and target requires OpenMP,
  # then link OpenMP libraries
  #
  if ((OpenMP_CXX_FOUND) AND (${t_name} IN_LIST omp_targets))
    target_link_libraries(_application-v${t_name} PUBLIC OpenMP::OpenMP_CXX)

    if (NOT SKIP_GSUITE)
      target_link_libraries(_test-v${t_name} PUBLIC OpenMP::OpenMP_CXX)
      target_link_libraries(_benchmark-v${t_name} PUBLIC OpenMP::OpenMP_CXX)
    endif()
  endif()

  # If target is dedicated to ITT the link ITT libraries and set compiler
  # definitions
  #
  if ((ITT_FOUND) AND (${t_name} IN_LIST itt_targets))
    target_compile_definitions(_application-v${t_name} PRIVATE ITT_TRACE)

    target_link_libraries(_application-v${t_name} PUBLIC ${ITT_LIBRARY})
  endif()

  if (NOT SKIP_GSUITE)
    # Discover Google Tests in the target
    #
    gtest_discover_tests(_test-v${t_name})
  endif()
endforeach()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
