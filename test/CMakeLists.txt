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