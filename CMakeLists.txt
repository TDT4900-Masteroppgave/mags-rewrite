cmake_minimum_required(VERSION 3.20)
project(mags_rewrite)

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mags_rewrite
        main.cpp
        candidate_generation.cpp
        candidate_generation.h
        mags_types.h)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)
add_executable(mags_tests tests.cpp candidate_generation.cpp)
target_link_libraries(mags_tests PRIVATE GTest::gtest_main GTest::gtest)