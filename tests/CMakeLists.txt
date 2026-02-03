cmake_minimum_required(VERSION 3.20)
project(mags_rewrite_tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(mags_tests
        candidate_generation_test.cpp
        GraphTestUtility.cpp
        GraphTestUtility.h
        DisjointSetUnionTest.cpp
        PartitionTest.cpp
        output_test.cpp
        preprocess_test.cpp
        file_util_test.cpp
        mags_test.cpp
)

target_link_libraries(mags_tests
        PRIVATE
        mags_logic
        GTest::gtest_main
        GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(mags_tests)