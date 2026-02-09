cmake_minimum_required(VERSION 3.20)
project(mags_rewrite)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (APPLE)
    execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(SYSTEM "${SDK_PATH}/usr/include/c++/v1")
endif ()

include(FetchContent)

# --- Fetch Dependencies
FetchContent_Declare(
        parallel_hashmap
        GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
        GIT_TAG v2.0.0
)
FetchContent_GetProperties(parallel_hashmap)
if (NOT parallel_hashmap_POPULATED)
    FetchContent_MakeAvailable(parallel_hashmap)
endif ()


# --- Sources / headers
set(MAGS_LOGIC_SOURCES
    src/candidate_generation.cpp
    src/DisjointSetUnion.cpp
    src/SuperNodeSet.cpp
    src/types.cpp
    src/greedy_merge.cpp
    src/output.cpp
    src/preprocess.cpp
    src/file_util.cpp
    src/mags.cpp
    src/graph_summarize.cpp
    src/util_time.cpp
)

set(MAGS_LOGIC_HEADERS
        include/mags/candidate_generation.h
        include/mags/types.h
        include/mags/DisjointSetUnion.h
        include/mags/output.h
        include/mags/preprocess.h
        include/mags/file_util.h
        include/mags/mags.h
        include/mags/SuperNodeSet.h
        include/mags/greedy_merge.h
        include/mags/util_time.h
        include/mags/graph_summarize.h
)

add_library(mags_logic ${MAGS_LOGIC_SOURCES} ${MAGS_LOGIC_HEADERS})
target_include_directories(mags_logic PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${parallel_hashmap_SOURCE_DIR}
)

# --- Executable
add_executable(mags_rewrite apps/main.cpp)
target_link_libraries(mags_rewrite PRIVATE mags_logic)

# ---- Tests
if(BUILD_TESTING)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
    )

    if (WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_MakeAvailable(googletest)

    target_compile_definitions(mags_logic PUBLIC UNIT_TESTING)

    target_link_libraries(mags_logic PUBLIC GTest::gtest)

    add_subdirectory(tests)
endif()

# --- Data Download
set(DOWNLOAD_SCRIPT "${CMAKE_SOURCE_DIR}/download_test_data.sh")

execute_process(
        COMMAND bash ${DOWNLOAD_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE DOWNLOAD_RESULT
)

if (NOT DOWNLOAD_RESULT EQUAL 0)
    message(WARNING "Failed to download test data. Tests requiring 'email-Eu-core.txt' will fail.")
endif ()
