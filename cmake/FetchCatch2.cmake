# FetchContent module to download Catch2 if not found
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # Latest stable version
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(Catch2)