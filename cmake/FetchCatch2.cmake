# FetchContent module to download Catch2 if not found
include(FetchContent)

# Ensure Catch2 uses the same runtime library as the main project (MSVC fix)
if(MSVC)
    # Set the runtime library for Catch2 to match the main project
    set(CMAKE_MSVC_RUNTIME_LIBRARY_BACKUP ${CMAKE_MSVC_RUNTIME_LIBRARY})
    # Force Catch2 to use the same runtime library
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
endif()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # Latest stable version
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(Catch2)

# Restore runtime library setting after fetching Catch2
if(MSVC AND CMAKE_MSVC_RUNTIME_LIBRARY_BACKUP)
    set(CMAKE_MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY_BACKUP})
endif()