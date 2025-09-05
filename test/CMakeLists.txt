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
        
        # For MSVC ASAN, ensure consistent runtime library settings
        if(MSVC AND BUSTACHE_ENABLE_SANITIZER_ADDRESS)
            set_property(TARGET ${TEST_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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