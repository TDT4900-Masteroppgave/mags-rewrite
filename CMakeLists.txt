cmake_minimum_required(VERSION 3.20)
project(mags_rewrite)
include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif()

# Core library to avoid redundant compilation in tests and main
add_library(cg_logic
        src/candidate_generation.cpp
        include/mags/candidate_generation.h
        include/mags/types.h
        src/util.cpp
        include/mags/util.h)

add_executable(mags_rewrite main.cpp)
target_link_libraries(mags_rewrite PRIVATE cg_logic)

include(FetchContent)

FetchContent_Declare(
        parallel_hashmap
        GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
        GIT_TAG        v2.0.0
)
FetchContent_MakeAvailable(parallel_hashmap)
target_link_libraries(cg_logic PUBLIC phmap)

target_include_directories(cg_logic PUBLIC include)
add_subdirectory(tests)