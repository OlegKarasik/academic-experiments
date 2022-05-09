cmake_minimum_required(VERSION 3.14.0)
project(application VERSION 0.1.0)

# Include a special Fetch Content command (to simplify usage of Google Tests)
#
include(FetchContent)

# Declare Google Tests content
#
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

# Make Google Tests available for the rest of the build
#
FetchContent_MakeAvailable(googletest)


add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

set(CMAKE_CXX_FLAGS "-std=c++17")

set(SRC_LIST src/_application.cpp)
set(INC_LIST ../_common; src/utilz)

set(TEST_SRC_LIST src/_test.cpp)

include_directories(${INC_LIST})

# Compile algorithms executables

add_executable(application-v0 ${SRC_LIST})
target_include_directories(application-v0 PUBLIC src/variants/00)

add_executable(application-v1 ${SRC_LIST})
target_include_directories(application-v1 PUBLIC src/variants/01)

add_executable(application-v2 ${SRC_LIST})
target_include_directories(application-v2 PUBLIC src/variants/02)

add_executable(_test-v0 ${TEST_SRC_LIST})
target_include_directories(_test-v0 PUBLIC src/variants/00)
target_link_libraries(_test-v0 PRIVATE GTest::GTest)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
