cmake_minimum_required(VERSION 3.14.0)
project(application VERSION 0.1.0)

# Include a special Fetch Content command (to simplify usage of Google Tests)
#
include(FetchContent)

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

# Make Google Tests and Google Benchmark available to the rest of the build
#
FetchContent_MakeAvailable(googletest googlebenchmark)

# Initialise Google Tests library entry point
#
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

# Set main compilation options
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure global includes for all configuration
# and targets
#
set(INC_LIST ../_common; src/utilz)

# Configure targets entry points
#
set(APP_SRC_LIST src/_application.cpp)
set(TST_SRC_LIST src/_test.cpp)
set(BNK_SRC_LIST src/_benchmark.cpp)

# Configure targets variations includes
#
set(VAR_00_INC_LIST src/variants/00)
set(VAR_01_INC_LIST src/variants/01)
set(VAR_02_INC_LIST src/variants/02)

# Initialise include directories
#
include_directories(${INC_LIST})

# Initialise targets
add_executable(application-v0 ${APP_SRC_LIST})
add_executable(application-v1 ${APP_SRC_LIST})
add_executable(application-v2 ${APP_SRC_LIST})
add_executable(_test-v0       ${TST_SRC_LIST})
add_executable(_test-v1       ${TST_SRC_LIST})
add_executable(_test-v2       ${TST_SRC_LIST})
add_executable(_benchmark-v0  ${BNK_SRC_LIST})
add_executable(_benchmark-v1  ${BNK_SRC_LIST})
add_executable(_benchmark-v2  ${BNK_SRC_LIST})

target_include_directories(application-v0 PRIVATE ${VAR_00_INC_LIST})
target_include_directories(application-v1 PRIVATE ${VAR_01_INC_LIST})
target_include_directories(application-v2 PRIVATE ${VAR_02_INC_LIST})
target_include_directories(_test-v0       PRIVATE ${VAR_00_INC_LIST})
target_include_directories(_test-v1       PRIVATE ${VAR_01_INC_LIST})
target_include_directories(_test-v2       PRIVATE ${VAR_02_INC_LIST})
target_include_directories(_benchmark-v0  PRIVATE ${VAR_00_INC_LIST})
target_include_directories(_benchmark-v1  PRIVATE ${VAR_01_INC_LIST})
target_include_directories(_benchmark-v2  PRIVATE ${VAR_02_INC_LIST})

target_link_libraries(_test-v0      PRIVATE GTest::GTest)
target_link_libraries(_test-v1      PRIVATE GTest::GTest)
target_link_libraries(_test-v2      PRIVATE GTest::GTest)
target_link_libraries(_benchmark-v0 PRIVATE benchmark::benchmark_main)
target_link_libraries(_benchmark-v1 PRIVATE benchmark::benchmark_main)
target_link_libraries(_benchmark-v2 PRIVATE benchmark::benchmark_main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
