cmake_minimum_required (VERSION 3.13)
project (rss-reader)

# Enable the C++23 standard
set (CMAKE_CXX_STANDARD 23)
include (FetchContent)

file (GLOB_RECURSE SRCS src/*.cpp)
add_executable (rss_reader ${SRCS})
target_include_directories (rss_reader PRIVATE include)
target_compile_options (rss_reader PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Declare the variable, containing all libraries which
# later will be linked.
set (libs_to_link)

# Fetch and populate dependencies
include (cmake/openssl.cmake)
include (cmake/spdlog.cmake)
include (cmake/boost.cmake)

target_link_libraries (rss_reader PRIVATE ${libs_to_link})

option (ENABLE_TESTS "Enable tests" OFF)
if (ENABLE_TESTS)
    include (cmake/googletest.cmake)
    # Build tests
    include (EnableTesting.cmake)
endif ()