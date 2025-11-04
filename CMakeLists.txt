cmake_minimum_required (VERSION 3.13)
project (rss-reader)

# Enable the C++23 standard
set (CMAKE_CXX_STANDARD 23)
include (FetchContent)

file (GLOB_RECURSE SRCS src/*.cpp)
add_executable (rss_reader ${SRCS})
target_include_directories (rss_reader PRIVATE include)
target_compile_options (rss_reader PRIVATE -Wall -Wextra -Wpedantic -Werror)

file (GLOB_RECURSE SRCS_HTTP src/http/*.cpp)
file (GLOB_RECURSE SRCS_UTILS src/utils/*.cpp)

set (CLIENT_SRCS)
list (APPEND CLIENT_SRCS ${SRCS_HTTP} ${SRCS_UTILS} src/ClientMain.cpp)
add_executable (http-client ${CLIENT_SRCS})
target_include_directories (http-client PRIVATE include)
target_compile_options (http-client PRIVATE -Wall -Wextra -Wpedantic -Werror)

set (SERVER_SRCS)
list (APPEND SERVER_SRCS ${SRCS_HTTP} ${SRCS_UTILS} src/ServerMain.cpp)
add_executable (http-server ${SERVER_SRCS})
target_include_directories (http-server PRIVATE include)
target_compile_options (http-server PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Declare the variable, containing all libraries which
# later will be linked.
set (libs_to_link)

# Fetch and populate dependencies
include (cmake/openssl.cmake)
include (cmake/spdlog.cmake)
include (cmake/boost.cmake)

target_link_libraries (rss_reader PRIVATE ${libs_to_link})
target_link_libraries (http-client PRIVATE ${libs_to_link})
target_link_libraries (http-server PRIVATE ${libs_to_link})

option (ENABLE_TESTS "Enable tests" OFF)
if (ENABLE_TESTS)
    include (cmake/googletest.cmake)
    # Build tests
    include (EnableTesting.cmake)
endif ()