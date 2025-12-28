enable_testing ()

set (TEST_DIR tests/src)
file (GLOB_RECURSE _TEST_SRCS ${TEST_DIR}/*.cpp)
set (TEST_SRCS ${_TEST_SRCS}
               src/http/HttpClient.cpp
               src/http/HttpRequest.cpp
               src/http/HttpResponse.cpp
               src/http/HttpConnection.cpp
               src/http/HttpConnectionFactory.cpp
               src/http/HttpHelpers.cpp
               src/utils/network/Socket.cpp
               src/utils/network/StatusCode.cpp
               src/utils/network/NetworkUtils.cpp
               src/utils/network/AddrInfoBuilder.cpp)

add_executable (
    rss_reader_tests
    ${TEST_SRCS}
)

target_include_directories (rss_reader_tests PRIVATE include)
target_include_directories (rss_reader_tests PRIVATE tests/include)
target_link_libraries (
    rss_reader_tests
    GTest::gmock_main
    spdlog
    Boost::algorithm
)

include (GoogleTest)
gtest_discover_tests (rss_reader_tests)