find_package(Catch2 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, fetching from GitHub...")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FetchCatch2.cmake)
endif()

# Additional safety check: ensure Catch2 targets have correct runtime library on MSVC
if(MSVC AND CMAKE_MSVC_RUNTIME_LIBRARY)
    foreach(target IN ITEMS Catch2 Catch2WithMain)
        if(TARGET ${target})
            get_target_property(current_runtime ${target} MSVC_RUNTIME_LIBRARY)
            if(NOT current_runtime STREQUAL CMAKE_MSVC_RUNTIME_LIBRARY)
                message(STATUS "Correcting MSVC runtime library for ${target}: ${current_runtime} -> ${CMAKE_MSVC_RUNTIME_LIBRARY}")
                set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
            else()
                message(STATUS "Catch2 target ${target} already has correct runtime library: ${current_runtime}")
            endif()
        endif()
    endforeach()
endif()

# This ensures Catch2 has the same ASAN annotations as test targets to prevent LNK2038 errors
if(MSVC AND BUSTACHE_ENABLE_SANITIZER_ADDRESS)
    message(STATUS "Applying AddressSanitizer to Catch2 targets (test-level fallback)")
    
    foreach(target IN ITEMS Catch2 Catch2WithMain)
        if(TARGET ${target})
            # Check if ASAN is already applied
            get_target_property(current_options ${target} COMPILE_OPTIONS)
            if(NOT current_options OR NOT "/fsanitize=address" IN_LIST current_options)
                target_compile_options(${target} PRIVATE /fsanitize=address)
                message(STATUS "Applied AddressSanitizer to ${target} (fallback)")
            else()
                message(STATUS "AddressSanitizer already applied to ${target}")
            endif()
        endif()
    endforeach()
endif()

function(add_catch_test name)
    set(TEST_TARGET test_${name})
    add_executable(${TEST_TARGET}
        ${name}.cpp
    )
    
    # Ensure C++ linker is used (prevents linking with C linker)
    set_target_properties(${TEST_TARGET} PROPERTIES LINKER_LANGUAGE CXX)
    
    target_link_libraries(${TEST_TARGET}
        ${PROJECT_NAME} Catch2::Catch2WithMain
    )
    target_compile_features(${TEST_TARGET} PUBLIC cxx_std_20)
    
    # Ensure consistent MSVC runtime library for all test targets
    if(MSVC)
        if(CMAKE_MSVC_RUNTIME_LIBRARY)
            set_property(TARGET ${TEST_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
        else()
            # Fallback to dynamic runtime if not set
            set_property(TARGET ${TEST_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
    
    # Apply developer settings to test targets if in developer mode
    if(BUSTACHE_DEVELOPER_MODE AND TARGET bustache_developer_settings)
        # Copy sanitizer and other developer settings to test targets
        get_target_property(DEV_COMPILE_OPTIONS bustache_developer_settings INTERFACE_COMPILE_OPTIONS)
        get_target_property(DEV_LINK_OPTIONS bustache_developer_settings INTERFACE_LINK_OPTIONS)
        get_target_property(DEV_COMPILE_DEFINITIONS bustache_developer_settings INTERFACE_COMPILE_DEFINITIONS)
        
        if(DEV_COMPILE_OPTIONS)
            target_compile_options(${TEST_TARGET} PRIVATE ${DEV_COMPILE_OPTIONS})
        endif()
        if(DEV_LINK_OPTIONS)
            target_link_options(${TEST_TARGET} PRIVATE ${DEV_LINK_OPTIONS})
        endif()
        if(DEV_COMPILE_DEFINITIONS)
            target_compile_definitions(${TEST_TARGET} PRIVATE ${DEV_COMPILE_DEFINITIONS})
        endif()
        
        # GCC 12 false positive workaround for -Warray-bounds in fmt/libstdc++ internals
        # This affects fmt v9's bigint/dragon formatting path used in large integer/float formatting
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
            target_compile_options(${TEST_TARGET} PRIVATE -Wno-error=array-bounds)
        endif()
        
        # GCC 13 false positive workaround for -Wmaybe-uninitialized in std::function move constructor
        # This affects std::variant containing std::function objects during move construction
        # See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109703
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13)
            target_compile_options(${TEST_TARGET} PRIVATE -Wno-error=maybe-uninitialized)
        endif()
        
        # For MSVC, ensure consistent runtime library settings
        if(MSVC)
            # Use the same runtime library as set in the main CMakeLists.txt
            # This prevents mixing static/dynamic CRTs which causes linker errors
            if(CMAKE_MSVC_RUNTIME_LIBRARY)
                set_property(TARGET ${TEST_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
            else()
                # Fallback to dynamic runtime if not set
                set_property(TARGET ${TEST_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
            endif()
        endif()
    endif()
    
    add_test(${TEST_TARGET} ${TEST_TARGET})
endfunction()

add_catch_test(specs)
add_catch_test(unresolved_handler)
add_catch_test(udt)
add_catch_test(inheritance)
add_catch_test(split_tag)
add_catch_test(dynamic_names)
add_catch_test(context_stack)
add_catch_test(boundary_tests)
add_catch_test(error_path_tests)
add_catch_test(failure_simulation_tests)
add_catch_test(custom_extensions_tests)
add_catch_test(dynamic_partials_tests)
add_catch_test(performance_tests)