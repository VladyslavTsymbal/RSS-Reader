FetchContent_Declare (
    openssl
    GIT_REPOSITORY https://github.com/openssl/openssl
    GIT_TAG        openssl-3.1.3
)
FetchContent_MakeAvailable (openssl)
FetchContent_GetProperties (openssl)
if (NOT openssl_POPULATED)
    message (FATAL_ERROR "`OpenSSL` is required to build application. Use FetchContent to populate it.")
endif ()