cmake_minimum_required(VERSION 3.20)
project(mags_rewrite_tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(mags_tests
        candidate_generation_test.cpp
        GraphTestUtility.cpp
        GraphTestUtility.h
        DisjointSetUnionTest.cpp
        PartitionTest.cpp
)

target_link_libraries(mags_tests
        PRIVATE
        mags_logic
        GTest::gtest_main
        GTest::gtest
)

target_compile_definitions(mags_logic PUBLIC UNIT_TESTING)

include(GoogleTest)
gtest_discover_tests(mags_tests)