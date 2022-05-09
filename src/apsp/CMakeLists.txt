cmake_minimum_required(VERSION 3.0.0)
project(application VERSION 0.1.0)

set (SRC_LIST shell/_application.cpp)

add_executable(application-v0 ${SRC_LIST})
add_executable(application-v1 ${SRC_LIST})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
