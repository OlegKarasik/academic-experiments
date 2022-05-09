cmake_minimum_required(VERSION 3.0.0)
project(application VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-std=c++17")

set(SRC_LIST src/_application.cpp)
set(INC_LIST ../_common; src/utilz)

include_directories(${INC_LIST})

add_executable(application-v0 ${SRC_LIST})
target_include_directories(application-v0 PUBLIC src/variants/00)

add_executable(application-v1 ${SRC_LIST})
target_include_directories(application-v1 PUBLIC src/variants/01)

add_executable(application-v2 ${SRC_LIST})
target_include_directories(application-v2 PUBLIC src/variants/02)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
