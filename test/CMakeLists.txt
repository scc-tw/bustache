find_package(Catch2 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, fetching from GitHub...")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FetchCatch2.cmake)
endif()

function(add_catch_test name)
    set(TEST_TARGET test_${name})
    add_executable(${TEST_TARGET}
        ${name}.cpp
    )
    target_link_libraries(${TEST_TARGET}
        ${PROJECT_NAME} Catch2::Catch2 Catch2::Catch2WithMain
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