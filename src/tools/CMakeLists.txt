cmake_minimum_required(VERSION 3.14.0)
project(tools VERSION 0.1.0)

# Set main compilation options
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure global includes for all configuration
# and targets
#
set(INC_LIST ../_common)

# Initialise include directories
#
include_directories(${INC_LIST})

# Initialise targets
add_executable(graphg   src/graphg.cpp)
add_executable(graphc   src/graphc.cpp)
add_executable(grapha   src/grapha.cpp)
add_executable(graphp   src/graphp.cpp)
add_executable(mempagei src/mempagei.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
