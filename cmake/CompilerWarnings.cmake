# Compiler warning configuration for bustache
# Based on Jason Turner's cpp_starter_project

function(set_project_warnings target)
    option(BUSTACHE_ENABLE_WARNINGS "Enable compiler warnings" ON)

    if(NOT BUSTACHE_ENABLE_WARNINGS)
        return()
    endif()

    set(MSVC_WARNINGS
        /W4          # Baseline reasonable warnings
        /WX          # Treat warnings as errors (always enabled)
        /permissive- # Standards conformance mode
        /w14242      # 'identifier': conversion from 'type1' to 'type1', possible loss of data
        /w14254      # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /w14263      # 'function': member function does not override any base class virtual member function
        /w14265      # 'classname': class has virtual functions, but destructor is not virtual
        /w14287      # 'operator': unsigned/negative constant mismatch
        /we4289      # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
        /w14296      # 'operator': expression is always 'boolean_value'
        /w14311      # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545      # expression before comma evaluates to a function which is missing an argument list
        /w14546      # function call before comma missing argument list
        /w14547      # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549      # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555      # expression has no effect; expected expression with side-effect
        /w14619      # pragma warning: there is no warning number 'number'
        /w14640      # Enable warning on thread un-safe static member initialization
        /w14826      # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
        /w14905      # wide string literal cast to 'LPSTR'
        /w14906      # string literal cast to 'LPWSTR'
        /w14928      # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        
        # Disable some warnings for external headers
        /wd4244      # 'conversion' conversion from 'type1' to 'type2', possible loss of data
        /wd4267      # 'var' : conversion from 'size_t' to 'type', possible loss of data
        /wd4996      # This function or variable may be unsafe
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Werror      # Treat warnings as errors (always enabled)
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    # Use PRIVATE for actual targets, INTERFACE for interface libraries
    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        target_compile_options(${target} INTERFACE ${PROJECT_WARNINGS})
    else()
        target_compile_options(${target} PRIVATE ${PROJECT_WARNINGS})
    endif()
endfunction()

# Function to disable warnings for specific targets (useful for tests or examples)
function(disable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /w)
    else()
        target_compile_options(${target} PRIVATE -w)
    endif()
endfunction()