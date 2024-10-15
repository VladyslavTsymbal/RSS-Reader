cmake_minimum_required (VERSION 3.13)
project (rss-reader)

# Enable the C++20 standard
set (CMAKE_CXX_STANDARD 20)

file (GLOB_RECURSE SRCS src/*.cpp)
add_executable (rss_reader ${SRCS})
target_include_directories (rss_reader PRIVATE include)
target_compile_options (rss_reader PRIVATE -Wall -Wextra -Wpedantic -Werror)

# TODO: Don't use from system. Add `external` dir and place external there
find_package(OpenSSL REQUIRED)
target_link_libraries (rss_reader PRIVATE OpenSSL::SSL)