# Sanitizer configuration for bustache
# Enable various sanitizers for debugging and testing

function(enable_sanitizers target)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        option(BUSTACHE_ENABLE_COVERAGE "Enable coverage reporting" OFF)
        option(BUSTACHE_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
        option(BUSTACHE_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(BUSTACHE_ENABLE_SANITIZER_UNDEFINED "Enable undefined behavior sanitizer" OFF)
        option(BUSTACHE_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
        option(BUSTACHE_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)

        set(SANITIZERS "")

        # Coverage
        if(BUSTACHE_ENABLE_COVERAGE)
            target_compile_options(${target} INTERFACE --coverage -O0 -g)
            target_link_options(${target} INTERFACE --coverage)
        endif()

        # Address sanitizer
        if(BUSTACHE_ENABLE_SANITIZER_ADDRESS)
            list(APPEND SANITIZERS "address")
        endif()

        # Leak sanitizer
        if(BUSTACHE_ENABLE_SANITIZER_LEAK)
            list(APPEND SANITIZERS "leak")
        endif()

        # Undefined behavior sanitizer
        if(BUSTACHE_ENABLE_SANITIZER_UNDEFINED)
            list(APPEND SANITIZERS "undefined")
        endif()

        # Thread sanitizer
        if(BUSTACHE_ENABLE_SANITIZER_THREAD)
            if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
                message(WARNING "Thread sanitizer does not work with Address or Leak sanitizer enabled")
            else()
                list(APPEND SANITIZERS "thread")
            endif()
        endif()

        # Memory sanitizer (Clang only)
        if(BUSTACHE_ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            if("address" IN_LIST SANITIZERS OR "thread" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
                message(WARNING "Memory sanitizer does not work with Address, Thread or Leak sanitizer enabled")
            else()
                list(APPEND SANITIZERS "memory")
            endif()
        endif()

        # Apply sanitizers
        list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)
        if(LIST_OF_SANITIZERS)
            if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
                target_compile_options(${target} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
                target_link_options(${target} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
            endif()
        endif()
    elseif(MSVC)
        option(BUSTACHE_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
        
        if(BUSTACHE_ENABLE_SANITIZER_ADDRESS)
            target_compile_options(${target} INTERFACE /fsanitize=address)
            
            # MSVC AddressSanitizer runtime library configuration
            # Note: ASAN on MSVC requires dynamic runtime library (MD/MDd) for proper operation
            # Static runtime (MT/MTd) can cause issues with ASAN's memory tracking
            if(NOT CMAKE_MSVC_RUNTIME_LIBRARY OR CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded$")
                message(STATUS "AddressSanitizer: Setting dynamic runtime library (MD/MDd) for optimal ASAN compatibility")
                set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
            else()
                # Use the globally configured runtime library
                set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY ${CMAKE_MSVC_RUNTIME_LIBRARY})
            endif()
        endif()
    endif()
endfunction()

# Function to print sanitizer configuration
function(print_sanitizer_config)
    message(STATUS "")
    message(STATUS "Sanitizer configuration:")
    message(STATUS "  Coverage:           ${BUSTACHE_ENABLE_COVERAGE}")
    message(STATUS "  Address Sanitizer:  ${BUSTACHE_ENABLE_SANITIZER_ADDRESS}")
    message(STATUS "  Leak Sanitizer:     ${BUSTACHE_ENABLE_SANITIZER_LEAK}")
    message(STATUS "  UB Sanitizer:       ${BUSTACHE_ENABLE_SANITIZER_UNDEFINED}")
    message(STATUS "  Thread Sanitizer:   ${BUSTACHE_ENABLE_SANITIZER_THREAD}")
    message(STATUS "  Memory Sanitizer:   ${BUSTACHE_ENABLE_SANITIZER_MEMORY}")
    message(STATUS "")
endfunction()