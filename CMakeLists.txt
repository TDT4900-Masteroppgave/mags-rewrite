cmake_minimum_required(VERSION 3.20)
project(mags_rewrite)

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mags_rewrite
        main.cpp
        candidate_generation.cpp
        candidate_generation.h
        mags_types.h)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0   # pick a known good release tag
)
FetchContent_MakeAvailable(googletest)
