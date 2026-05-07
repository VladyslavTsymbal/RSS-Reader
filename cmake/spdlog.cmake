FetchContent_Declare (
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.15.0
    GIT_SHALLOW TRUE
)

set(_RSS_READER_CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD})
set(CMAKE_CXX_STANDARD 17)
FetchContent_MakeAvailable (spdlog)
set(CMAKE_CXX_STANDARD ${_RSS_READER_CMAKE_CXX_STANDARD})
unset(_RSS_READER_CMAKE_CXX_STANDARD)

FetchContent_GetProperties (spdlog)
if (NOT spdlog_POPULATED)
    message (FATAL_ERROR "`spdlog` is required to build application. Use FetchContent to populate it.")
endif ()

list(APPEND libs_to_link "spdlog")
