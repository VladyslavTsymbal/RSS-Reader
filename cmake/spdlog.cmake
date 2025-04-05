FetchContent_Declare (
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.15.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable (spdlog)
FetchContent_GetProperties (spdlog)
if (NOT spdlog_POPULATED)
    message (FATAL_ERROR "`spdlog` is required to build application. Use FetchContent to populate it.")
endif ()

list(APPEND libs_to_link "spdlog")