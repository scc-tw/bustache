# Standard project settings for bustache
# This file contains standard settings that should be applied to all targets

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Generate compile_commands.json for clang-based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable folder structure in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set C++ standard requirements
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable IPO/LTO if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
    option(BUSTACHE_ENABLE_IPO "Enable Interprocedural Optimization (LTO)" OFF)
else()
    message(STATUS "IPO/LTO not supported: ${IPO_ERROR}")
endif()

# RPATH settings for installed binaries
if(NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
else()
    set(CMAKE_INSTALL_RPATH @loader_path)
endif()

# Windows-specific settings
if(WIN32)
    add_compile_definitions(
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Enable ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    option(BUSTACHE_ENABLE_CCACHE "Enable ccache" ON)
    if(BUSTACHE_ENABLE_CCACHE)
        set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
        set(CMAKE_C_COMPILER_LAUNCHER ccache)
    endif()
endif()