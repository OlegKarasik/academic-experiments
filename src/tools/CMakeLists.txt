cmake_minimum_required(VERSION 3.14.0)
project(tools VERSION 0.1.0)

# Set main compilation options
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Open MP support
#
find_package(OpenMP)

# Configure global includes for all configuration
# and targets
#
set(INC_LIST ../_common)

# Initialise include directories
#
include_directories(${INC_LIST})

# Initialise targets
add_executable(graphg   src/graphg.cpp)
add_executable(graphm   src/graphm.cpp)
add_executable(grapha   src/grapha.cpp)
add_executable(graphp   src/graphp.cpp)
add_executable(mempagei src/mempagei.cpp)

if(OpenMP_CXX_FOUND)
  target_link_libraries(graphg PUBLIC OpenMP::OpenMP_CXX)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
