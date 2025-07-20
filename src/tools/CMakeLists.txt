cmake_minimum_required(VERSION 3.14.0)
project(tools VERSION 0.1.0)

# Set main compilation options
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure global includes for all configuration
# and targets
#
set(INC_LIST ../_common)

# Initialise include directories
#
include_directories(${INC_LIST})

# Include global compile options
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options("-fno-diagnostics-color")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options("-g" "-march=native" "-mtune=native")
  endif()
endif()

# Initialise targets
#
add_executable(graphg   src/graphg.cpp)
add_executable(graphc   src/graphc.cpp)
add_executable(grapha   src/grapha.cpp)

if (WIN32)
  add_executable(mempagei src/mempagei.cpp)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
