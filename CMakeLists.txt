cmake_minimum_required (VERSION 3.13)
project (rss-reader)

# Enable the C++20 standard
set (CMAKE_CXX_STANDARD 20)
include (FetchContent)

file (GLOB_RECURSE SRCS src/*.cpp)
add_executable (rss_reader ${SRCS})
target_include_directories (rss_reader PRIVATE include)
target_compile_options (rss_reader PRIVATE -Wall -Wextra -Wpedantic -Werror)

# TODO: Don't use from system. Add `external` dir and place externals there
find_package(OpenSSL REQUIRED)
target_link_libraries (rss_reader PRIVATE OpenSSL::SSL)

option (ENABLE_TESTS "Enable Utils tests" OFF)
if (ENABLE_TESTS)
    FetchContent_Declare (
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    FetchContent_MakeAvailable (googletest)
    FetchContent_GetProperties (googletest)
    if (NOT googletest_POPULATED)
        message (FATAL_ERROR "GoogleTest is required to build rss-reader tests. Use FetchContent to populate it.")
    endif ()
    # Build tests
    include (EnableTesting.cmake)
endif ()