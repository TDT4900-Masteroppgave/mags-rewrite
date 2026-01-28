cmake_minimum_required(VERSION 3.20)
project(mags_rewrite)
include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif()

set(MAGS_LOGIC_SOURCES
        src/candidate_generation.cpp
        src/util.cpp
        src/DisjointSetUnion.cpp
        src/Partition.cpp
)

set(MAGS_LOGIC_HEADERS
        include/mags/candidate_generation.h
        include/mags/types.h
        include/mags/util.h
        include/mags/DisjointSetUnion.h
        include/mags/Partition.h
)

add_library(mags_logic ${MAGS_LOGIC_SOURCES} ${MAGS_LOGIC_HEADERS})

add_executable(mags_rewrite main.cpp)
target_link_libraries(mags_rewrite PRIVATE mags_logic)

include(FetchContent)
FetchContent_Declare(
        parallel_hashmap
        GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
        GIT_TAG v2.0.0
)
FetchContent_MakeAvailable(parallel_hashmap)
target_link_libraries(mags_logic PUBLIC phmap)

target_include_directories(mags_logic PUBLIC include)
add_subdirectory(tests)
