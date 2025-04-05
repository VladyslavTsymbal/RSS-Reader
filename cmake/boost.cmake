set(BOOST_INCLUDE_LIBRARIES algorithm)
set(BOOST_ENABLE_CMAKE ON)

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.85.0
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(Boost)
if(NOT Boost_POPULATED)
    message(STATUS "Fetching Boost...")
    FetchContent_MakeAvailable(Boost)
else()
    message(STATUS "Boost is already fetched and available.")
endif()

list(APPEND libs_to_link "Boost::algorithm")
