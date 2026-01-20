cmake_minimum_required(VERSION 3.20)
project(mags_rewrite_tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# 1. Ensure the source file path is correct relative to this folder
add_executable(mags_tests
        candidate_generation_test.cpp
        GraphTestUtility.cpp
        GraphTestUtility.h)

# 2. Use 'cg_logic' to match your root library name
target_link_libraries(mags_tests
        PRIVATE
        cg_logic
        GTest::gtest_main
        GTest::gtest
)

# 3. Discover individual tests for the CLion UI
include(GoogleTest)
gtest_discover_tests(mags_tests)