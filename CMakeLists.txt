# =============================================================================
# Bustache - C++20 Mustache Template Library
# =============================================================================
cmake_minimum_required(VERSION 3.15...3.27)

# =============================================================================
# Project Declaration
# =============================================================================
project(bustache
    VERSION 0.1.0
    DESCRIPTION "C++20 implementation of Mustache templating"
    HOMEPAGE_URL "https://github.com/scc-tw/bustache"
    LANGUAGES CXX
)

# Require out-of-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# =============================================================================
# CMake Modules and Settings
# =============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include custom modules
include(StandardProjectSettings)
include(CompilerWarnings)
include(Sanitizers)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# =============================================================================
# Project Options
# =============================================================================
option(BUSTACHE_BUILD_LIBRARY "Build bustache as a library (vs header-only)" ON)
option(BUSTACHE_ENABLE_TESTING "Enable testing of the bustache library" OFF)
option(BUSTACHE_ENABLE_EXAMPLES "Build example programs" OFF)
option(BUSTACHE_ENABLE_DOCS "Build documentation" OFF)
option(BUSTACHE_USE_FMT "Use fmt library instead of std::format" OFF)
option(BUSTACHE_INSTALL "Generate installation target" ON)
option(BUSTACHE_ENABLE_CACHE "Enable compilation cache (ccache/sccache)" ON)

# Developer options
option(BUSTACHE_DEVELOPER_MODE "Enable developer mode (warnings, sanitizers, etc.)" OFF)

# Print configuration
message(STATUS "")
message(STATUS "=== Bustache Configuration ===")
message(STATUS "Version:           ${PROJECT_VERSION}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      C++20 (required)")
message(STATUS "Build library:     ${BUSTACHE_BUILD_LIBRARY}")
message(STATUS "Testing:           ${BUSTACHE_ENABLE_TESTING}")
message(STATUS "Examples:          ${BUSTACHE_ENABLE_EXAMPLES}")
message(STATUS "Documentation:     ${BUSTACHE_ENABLE_DOCS}")
message(STATUS "Use fmt:           ${BUSTACHE_USE_FMT}")
message(STATUS "Installation:      ${BUSTACHE_INSTALL}")
message(STATUS "Developer mode:    ${BUSTACHE_DEVELOPER_MODE}")
message(STATUS "")

# =============================================================================
# Dependencies
# =============================================================================
# Check for C++20 compiler support
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "GCC 10+ required for C++20 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "Clang 10+ required for C++20 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
        message(FATAL_ERROR "Apple Clang 12+ (Xcode 12+) required for C++20 support")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.29)
        message(FATAL_ERROR "MSVC 2019 16.10+ required for C++20 support")
    endif()
endif()

# fmt library (optional)
if(BUSTACHE_USE_FMT)
    find_package(fmt 8.0 REQUIRED)
    message(STATUS "Using fmt library version ${fmt_VERSION}")
endif()

# =============================================================================
# Interface Library (Header-only mode)
# =============================================================================
add_library(bustache_headers INTERFACE)
add_library(bustache::headers ALIAS bustache_headers)

target_compile_features(bustache_headers INTERFACE cxx_std_20)
target_include_directories(bustache_headers 
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(BUSTACHE_USE_FMT)
    target_link_libraries(bustache_headers INTERFACE fmt::fmt)
    target_compile_definitions(bustache_headers INTERFACE BUSTACHE_USE_FMT)
endif()

# =============================================================================
# Main Library Target
# =============================================================================
if(BUSTACHE_BUILD_LIBRARY)
    add_library(bustache
        src/format.cpp
        src/render.cpp
    )
    add_library(bustache::bustache ALIAS bustache)

    # Set library properties
    set_target_properties(bustache PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    # Library configuration
    target_compile_features(bustache PUBLIC cxx_std_20)
    
    target_include_directories(bustache
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Link dependencies
    if(BUSTACHE_USE_FMT)
        target_link_libraries(bustache PUBLIC fmt::fmt)
        target_compile_definitions(bustache PUBLIC BUSTACHE_USE_FMT)
    endif()

    # Export symbols for shared library
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(bustache 
            PRIVATE BUSTACHE_EXPORT 
            INTERFACE BUSTACHE_SHARED
        )
    endif()

    # Apply IPO/LTO if enabled
    if(BUSTACHE_ENABLE_IPO AND IPO_SUPPORTED)
        set_property(TARGET bustache PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    
    # Apply compiler warnings (including warnings as errors by default)
    set_project_warnings(bustache)
else()
    # Header-only mode
    add_library(bustache ALIAS bustache_headers)
    add_library(bustache::bustache ALIAS bustache_headers)
endif()

# =============================================================================
# Developer Mode Settings
# =============================================================================
if(BUSTACHE_DEVELOPER_MODE)
    # Create interface library for developer settings (not exported)
    add_library(bustache_developer_settings INTERFACE)
    
    # Add compiler warnings
    set_project_warnings(bustache_developer_settings)
    
    # Enable sanitizers
    enable_sanitizers(bustache_developer_settings)
    
    # Apply developer settings directly to targets instead of linking
    # This avoids export issues
    if(BUSTACHE_BUILD_LIBRARY)
        get_target_property(DEV_COMPILE_OPTIONS bustache_developer_settings INTERFACE_COMPILE_OPTIONS)
        get_target_property(DEV_LINK_OPTIONS bustache_developer_settings INTERFACE_LINK_OPTIONS)
        get_target_property(DEV_COMPILE_DEFINITIONS bustache_developer_settings INTERFACE_COMPILE_DEFINITIONS)
        
        if(DEV_COMPILE_OPTIONS)
            target_compile_options(bustache PRIVATE ${DEV_COMPILE_OPTIONS})
        endif()
        if(DEV_LINK_OPTIONS)
            target_link_options(bustache PRIVATE ${DEV_LINK_OPTIONS})
        endif()
        if(DEV_COMPILE_DEFINITIONS)
            target_compile_definitions(bustache PRIVATE ${DEV_COMPILE_DEFINITIONS})
        endif()
    endif()
    
    # Print sanitizer configuration
    print_sanitizer_config()
endif()

# =============================================================================
# Testing
# =============================================================================
if(BUSTACHE_ENABLE_TESTING)
    enable_testing()
    include(CTest)
    add_subdirectory(test)
endif()

# =============================================================================
# Examples
# =============================================================================
if(BUSTACHE_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# Documentation
# =============================================================================
if(BUSTACHE_ENABLE_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # Configure Doxygen
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        
        doxygen_add_docs(docs
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/README.md
            COMMENT "Generating API documentation with Doxygen"
        )
    else()
        message(WARNING "Doxygen not found, documentation will not be built")
    endif()
endif()

# =============================================================================
# Installation
# =============================================================================
if(BUSTACHE_INSTALL)
    # Determine targets to install
    set(BUSTACHE_INSTALL_TARGETS bustache_headers)
    if(BUSTACHE_BUILD_LIBRARY)
        list(APPEND BUSTACHE_INSTALL_TARGETS bustache)
    endif()

    # Install targets
    install(
        TARGETS ${BUSTACHE_INSTALL_TARGETS}
        EXPORT bustache-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install headers
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/bustache
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Install export targets
    install(
        EXPORT bustache-targets
        FILE bustache-targets.cmake
        NAMESPACE bustache::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bustache
    )

    # Generate and install package configuration files
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bustache-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/bustache-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bustache
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/bustache-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/bustache-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/bustache-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bustache
    )

    # Export for build tree
    export(
        EXPORT bustache-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/bustache-targets.cmake
        NAMESPACE bustache::
    )

    # Register package in user package registry
    export(PACKAGE bustache)
endif()

# =============================================================================
# CPack Configuration (Packaging)
# =============================================================================
if(BUSTACHE_INSTALL)
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_VENDOR "Bustache Contributors")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
    set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
    # Set license and readme files if they exist
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE_1_0.txt")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE_1_0.txt")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
        set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    endif()

    # Source package
    set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
    set(CPACK_SOURCE_IGNORE_FILES
        /.git
        /build
        /.cache
        /.vscode
        /.idea
        ".*~$"
    )

    include(CPack)
endif()

# =============================================================================
# Summary
# =============================================================================
if(BUSTACHE_BUILD_LIBRARY)
    message(STATUS "Bustache library will be built")
else()
    message(STATUS "Bustache configured as header-only")
endif()

if(BUSTACHE_DEVELOPER_MODE)
    message(STATUS "Developer mode enabled - extra warnings and sanitizers available")
endif()

message(STATUS "Configuration complete!")
message(STATUS "")