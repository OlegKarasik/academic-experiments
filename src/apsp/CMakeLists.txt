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

# Find Metal if needed (MacOS)
#
if (APPLE)
  set(METAL_LIBS "-framework Metal" "-framework MetalKit" "-framework AppKit" "-framework Foundation" "-framework QuartzCore")
endif()

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
    QUERY WINDOWS_REGISTRY "HKLM\\SOFTWARE\\Oleg Karasik\\AK-Kernel\\1.31.0"
    VALUE "InstallPath")

  find_path(KERNEL_INCLUDE_DIRS Kernel{Core}.h
    PATHS ${CMAKE_KERNEL_ROOT_PATH}
    PATH_SUFFIXES include)

  find_path(KERNEL_LIBRARY_DIRS Kernel.Core.lib
    PATHS ${CMAKE_KERNEL_ROOT_PATH}
    PATH_SUFFIXES lib)

  find_path(KERNEL_BINARY_DIRS Kernel.Core.dll
    PATHS ${CMAKE_KERNEL_ROOT_PATH}
    PATH_SUFFIXES bin)

  if(NOT KERNEL_INCLUDE_DIRS MATCHES NOTFOUND AND
     NOT KERNEL_LIBRARY_DIRS MATCHES NOTFOUND AND
     NOT KERNEL_BINARY_DIRS  MATCHES NOTFOUND)

    message(STATUS "Found AK-Kernel includes in ${KERNEL_INCLUDE_DIRS}")
    message(STATUS "Found AK-Kernel libraries in ${KERNEL_LIBRARY_DIRS}")
    message(STATUS "Found AK-Kernel binaries in ${KERNEL_BINARY_DIRS}")

    include_directories(${KERNEL_INCLUDE_DIRS})
    link_directories(${KERNEL_LIBRARY_DIRS})

    set(KERNEL_LIBRARY "Kernel.Core.lib")

    set(KERNEL_DLL         "${KERNEL_BINARY_DIRS}/Kernel.Core.dll")
    set(KERNEL_DLL_SYMBOLS "${KERNEL_BINARY_DIRS}/Kernel.Core.pdb")

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
    message(STATUS "Found Visual Studio in ${VS_INSTALL_PATH}")

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

# Fix some of the compilation options, especially default Windows version.
# In the recent version of MinGW the NTDDI_VERSION version detection mechanism
# has changed and now dependencies are build for default version
# of Windows 0x6.
#
if (WIN32)
  add_compile_definitions("NTDDI_VERSION=0x0A000010")
endif()

if (NOT SKIP_GSUITE)
  # Declare Google Tests and Google Benchmark content
  #
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
  )

  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.1
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
set(INC_LIST ../_common; src/utilz; ../_deps)

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

# Initialise algorithms targets
#
list(APPEND targets_names "00")
list(APPEND targets_names "01")
list(APPEND targets_names "02")
list(APPEND targets_names "03")
list(APPEND targets_names "04")
list(APPEND targets_names "07")
list(APPEND targets_names "08")

# Initialise Metal specific targets (MacOS)
#
if (APPLE)
  list(APPEND metal_targets "06-metal")
endif()

# Initialise Kernel compatible targets (Windows & Kernel)
#
if (WIN32 AND KERNEL_FOUND AND VS_FOUND)
  list(APPEND kernel_targets "05-kernel")
endif()

# Initialise OpenMP compatible targets (OpenMP)
#
if (OpenMP_CXX_FOUND)
  list(APPEND omp_targets "00-omp")
  list(APPEND omp_targets "01-omp")
  list(APPEND omp_targets "02-omp")
  list(APPEND omp_targets "03-omp")
  list(APPEND omp_targets "04-omp")
  list(APPEND omp_targets "07-omp")
  list(APPEND omp_targets "08-omp")
endif()

# Initialise ITT targets if ITT is enabled
#
if (ITT_FOUND)
  if (APPLE)
    list(APPEND itt_metal_targets "06-metal-itt")
  endif()

  if (WIN32 AND KERNEL_FOUND AND VS_FOUND)
    # Include all Kernel targets from kernel_targets into -itt targets
    #
    list(LENGTH kernel_targets _init_kernel_itt_targets_length)
    math(EXPR init_kernel_itt_targets_length "${_init_kernel_itt_targets_length}-1")
    foreach(index RANGE ${init_kernel_itt_targets_length})
      list(GET kernel_targets ${index} t_name)

      list(APPEND itt_kernel_targets "${t_name}-itt")
    endforeach()
  endif()

  if (OpenMP_CXX_FOUND)
    # Include all OpenMP targets from omp_targets into -itt targets
    #
    list(LENGTH omp_targets _init_omp_itt_targets_length)
    math(EXPR init_omp_itt_targets_length "${_init_omp_itt_targets_length}-1")
    foreach(index RANGE ${init_omp_itt_targets_length})
      list(GET omp_targets ${index} t_name)

      list(APPEND itt_omp_targets "${t_name}-itt")
    endforeach()
  endif()

  # Include all targets from targets_names into -itt targets
  #
  list(LENGTH targets_names _init_itt_targets_length)
  math(EXPR init_itt_targets_length "${_init_itt_targets_length}-1")
  foreach(index RANGE ${init_itt_targets_length})
    list(GET targets_names ${index} t_name)

    list(APPEND itt_targets "${t_name}-itt")
  endforeach()

  if (itt_omp_targets)
    list(APPEND itt_targets "${itt_omp_targets}")
    list(APPEND omp_targets "${itt_omp_targets}")
  endif()
  if (itt_kernel_targets)
    list(APPEND itt_targets "${itt_kernel_targets}")
    list(APPEND kernel_targets "${itt_kernel_targets}")
  endif()
  if (itt_metal_targets)
    list(APPEND itt_targets "${itt_metal_targets}")
    list(APPEND targets_names "${itt_metal_targets}")
  endif()
endif()

if (omp_targets)
  list(APPEND targets_names "${omp_targets}")
endif()
if (kernel_targets)
  list(APPEND targets_names "${kernel_targets}")
endif()
if (metal_targets)
  list(APPEND targets_names "${metal_targets}")
endif()

add_custom_target(tests)

# Initialise length
#
list(LENGTH targets_names _length)
math(EXPR length "${_length}-1")

# Initialise targets
foreach(index RANGE ${length})
  # Here we initialise `t_name` -- target name
  #
  list(GET targets_names ${index} t_name)

  # Here we parse the target name into an alias -- `t_alias`
  #
  string(FIND "${t_name}" "-" position)
  if (position LESS 1)
    set(t_alias "${t_name}")
  else ()
    string(SUBSTRING "${t_name}" 0 "${position}" t_alias)
  endif ()

  # Here if tests are enabled we initialize name of the
  # test fixture -- `t_fixture`
  #
  if (NOT SKIP_GSUITE)
    string(REPLACE - _ t_fixture ${t_name})
  endif()

  # Making a targeted copies of the targets to ensure
  # they might include additional *.cpp to compile
  #
  set(t_app_src_list "${APP_SRC_LIST}")
  set(t_tst_src_list "${TST_SRC_LIST}")
  set(t_bnk_src_list "${BNK_SRC_LIST}")

  # Include Metal.cpp into compilation list if the target is Metal
  #
  if ((APPLE) AND (${t_name} IN_LIST metal_targets))
    list(APPEND t_app_src_list ../_deps/metal-cpp/Metal.cpp)
    list(APPEND t_tst_src_list ../_deps/metal-cpp/Metal.cpp)
    list(APPEND t_bnk_src_list ../_deps/metal-cpp/Metal.cpp)
  endif()

  add_executable(_application-v${t_name} ${t_app_src_list})

  target_include_directories(_application-v${t_name} PRIVATE src/variants/${t_alias})

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(_application-v${t_name} PRIVATE "-Wall")
  endif()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(_application-v${t_name} PRIVATE "-Wall")
  endif()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(_application-v${t_name} PRIVATE "/W4")
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(_application-v${t_name} PRIVATE "-g" "-march=native" "-mtune=native")
    endif()
  endif()

  if (NOT SKIP_GSUITE)
    add_executable(_test-v${t_name}        ${t_tst_src_list})
    add_executable(_benchmark-v${t_name}   ${t_bnk_src_list})

    add_dependencies(tests _test-v${t_name})

    target_include_directories(_test-v${t_name}        PRIVATE src/variants/${t_alias})
    target_include_directories(_benchmark-v${t_name}   PRIVATE src/variants/${t_alias})

    target_link_libraries(_test-v${t_name}      PRIVATE GTest::gtest_main)
    target_link_libraries(_benchmark-v${t_name} PRIVATE benchmark::benchmark_main)

    target_compile_definitions(_test-v${t_name} PRIVATE FIXTURE_NAME=${t_fixture})

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(_benchmark-v${t_name} PRIVATE "-fopt-info-vec-optimized" "-fopt-info-loop-optimized" "-save-temps" "-fverbose-asm")
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      target_compile_options(_benchmark-v${t_name} PRIVATE "-Rpass=loop-vectorize")
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
    file(COPY ${KERNEL_DLL_SYMBOLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
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

  # If target is dedicated to Metal the link Metal libraries and set compiler
  # definitions
  #
  if ((APPLE) AND (${t_name} IN_LIST metal_targets))
    target_link_libraries(_application-v${t_name} PUBLIC ${METAL_LIBS})

    add_custom_command(
      OUTPUT _algorithm-v${t_name}.metallib
      COMMAND xcrun -sdk macosx metal -c ${CMAKE_SOURCE_DIR}/src/variants/${t_alias}/algorithm.metal -o ${CMAKE_BINARY_DIR}/_algorithm-v${t_name}.air
      COMMAND xcrun -sdk macosx metallib ${CMAKE_BINARY_DIR}/_algorithm-v${t_name}.air -o ${CMAKE_BINARY_DIR}/_algorithm-v${t_name}.metallib
      DEPENDS ${CMAKE_SOURCE_DIR}/src/variants/${t_alias}/algorithm.metal
      VERBATIM)

    add_custom_target(_algorithm-v${t_name}-metallib ALL DEPENDS _algorithm-v${t_name}.metallib)

    add_dependencies(_application-v${t_name} _algorithm-v${t_name}-metallib)

    if (NOT SKIP_GSUITE)
      target_link_libraries(_test-v${t_name} PUBLIC ${METAL_LIBS})
      target_link_libraries(_benchmark-v${t_name} PUBLIC ${METAL_LIBS})

      add_dependencies(_test-v${t_name} _algorithm-v${t_name}-metallib)
      add_dependencies(_benchmark-v${t_name} _algorithm-v${t_name}-metallib)
    endif()
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
