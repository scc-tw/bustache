# FetchContent module to download Catch2 if not found
include(FetchContent)

# Ensure Catch2 uses the same runtime library as the main project (MSVC fix)
if(MSVC)
    # Set the runtime library for Catch2 to match the main project
    set(CMAKE_MSVC_RUNTIME_LIBRARY_BACKUP ${CMAKE_MSVC_RUNTIME_LIBRARY})
    
    # Force Catch2 to use the same runtime library by setting the policy
    # This must be set BEFORE FetchContent_Declare
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    
    # Explicitly set the runtime library for all targets in the Catch2 subproject
    # This ensures consistency across all Catch2 targets
    if(CMAKE_MSVC_RUNTIME_LIBRARY)
        message(STATUS "Configuring Catch2 with MSVC runtime library: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    else()
        # Default to dynamic runtime if not explicitly set
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        message(STATUS "Configuring Catch2 with default MSVC runtime library: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    endif()
endif()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # Latest stable version
    GIT_SHALLOW    TRUE
)

# Set Catch2 build options before making it available
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "")
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "")
set(BUILD_TESTING OFF CACHE BOOL "")

# Ensure Catch2 is built with PIC to be compatible with PIE executables
set(CMAKE_POSITION_INDEPENDENT_CODE_BACKUP ${CMAKE_POSITION_INDEPENDENT_CODE})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

FetchContent_MakeAvailable(Catch2)

# After Catch2 is available, ensure all Catch2 targets have PIC enabled for PIE compatibility
foreach(target IN ITEMS Catch2 Catch2WithMain)
    if(TARGET ${target})
        set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
        message(STATUS "Set POSITION_INDEPENDENT_CODE=ON for Catch2 target ${target}")
    endif()
endforeach()

# After Catch2 is available, explicitly set the runtime library for all Catch2 targets
# This is a belt-and-suspenders approach to ensure consistency
if(MSVC AND CMAKE_MSVC_RUNTIME_LIBRARY)
    # Get all targets created by Catch2
    get_property(CATCH2_TARGETS DIRECTORY "${catch2_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    
    foreach(target IN LISTS CATCH2_TARGETS)
        if(TARGET ${target})
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
            message(STATUS "Set MSVC runtime library for Catch2 target ${target}: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
        endif()
    endforeach()
    
    # Also explicitly set for known Catch2 targets
    foreach(target IN ITEMS Catch2 Catch2WithMain)
        if(TARGET ${target})
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
            message(STATUS "Explicitly set MSVC runtime library for ${target}: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
        endif()
    endforeach()
endif()

# CRITICAL: Apply AddressSanitizer settings to Catch2 targets to prevent annotation mismatches
# When ASAN is enabled, it adds annotations to std::string and std::vector that must be consistent
# across all linked components to avoid LNK2038 errors on MSVC
if(MSVC AND BUSTACHE_ENABLE_SANITIZER_ADDRESS)
    message(STATUS "Applying AddressSanitizer settings to Catch2 targets to prevent annotation mismatches")
    
    # Apply ASAN compile options to all Catch2 targets
    foreach(target IN ITEMS Catch2 Catch2WithMain)
        if(TARGET ${target})
            target_compile_options(${target} PRIVATE /fsanitize=address)
            message(STATUS "Applied AddressSanitizer to Catch2 target ${target}")
        endif()
    endforeach()
    
    # Also apply to any other Catch2 targets that might exist
    get_property(CATCH2_TARGETS DIRECTORY "${catch2_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS CATCH2_TARGETS)
        if(TARGET ${target})
            target_compile_options(${target} PRIVATE /fsanitize=address)
            message(STATUS "Applied AddressSanitizer to Catch2 target ${target}")
        endif()
    endforeach()
endif()

# Restore settings after fetching Catch2 (if they were backed up)
if(MSVC AND CMAKE_MSVC_RUNTIME_LIBRARY_BACKUP)
    set(CMAKE_MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY_BACKUP})
endif()

if(CMAKE_POSITION_INDEPENDENT_CODE_BACKUP)
    set(CMAKE_POSITION_INDEPENDENT_CODE ${CMAKE_POSITION_INDEPENDENT_CODE_BACKUP})
endif()