enable_testing ()

set (TEST_DIR tests/src)
file (GLOB_RECURSE _TEST_SRCS ${TEST_DIR}/*.cpp)
set (TEST_SRCS ${_TEST_SRCS}
               src/http/HttpClient.cpp
               src/http/HttpRequest.cpp
               src/http/HttpConnection.cpp
               src/http/HttpConnectionFactory.cpp
               src/utils/network/NetworkUtils.cpp
               src/utils/network/AddrInfoBuilder.cpp)

add_executable (
    rss-reader-tests
    ${TEST_SRCS}
)

target_include_directories (rss-reader-tests PRIVATE include)
target_include_directories (rss-reader-tests PRIVATE tests/include)
target_link_libraries (
    rss-reader-tests
    GTest::gmock_main
    spdlog
)

include (GoogleTest)
gtest_discover_tests (rss-reader-tests)