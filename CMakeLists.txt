cmake_minimum_required(VERSION 3.20)
project(mags_rewrite)

# Best practice: Set CXX standard before any targets
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif()

# Core library to avoid redundant compilation in tests and main
add_library(cg_logic candidate_generation.cpp candidate_generation.h types.h
        util.cpp
        util.h)

add_executable(mags_rewrite main.cpp
        util.cpp
        util.h)
target_link_libraries(mags_rewrite PRIVATE cg_logic)

include(FetchContent)

FetchContent_Declare(
        parallel_hashmap
        GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
        GIT_TAG        v2.0.0
)
FetchContent_MakeAvailable(parallel_hashmap)
target_link_libraries(cg_logic PRIVATE phmap)


FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(mags_tests candidate_generation_test.cpp)
target_link_libraries(mags_tests PRIVATE cg_logic GTest::gtest_main GTest::gtest)