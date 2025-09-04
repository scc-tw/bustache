# Using Bustache in Your Project

This guide demonstrates how to use bustache in external projects with the new professional CMake configuration.

## Method 1: Using find_package (After Installation)

### Step 1: Install bustache
```bash
# Build and install bustache
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build . --config Release
cmake --install . --config Release
```

### Step 2: Use in your CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(my_app)

# Find bustache
find_package(bustache REQUIRED)

# Create your executable
add_executable(my_app main.cpp)

# Link with bustache (library version)
target_link_libraries(my_app PRIVATE bustache::bustache)

# OR for header-only usage
# target_link_libraries(my_app PRIVATE bustache::headers)
```

## Method 2: Using FetchContent (Direct Integration)

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_app)

include(FetchContent)

# Fetch bustache directly from repository
FetchContent_Declare(
    bustache
    GIT_REPOSITORY https://github.com/scc-tw/bustache.git
    GIT_TAG        main
)

# Configure bustache options before making it available
set(BUSTACHE_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BUSTACHE_BUILD_LIBRARY ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bustache)

# Create your executable
add_executable(my_app main.cpp)

# Link with bustache
target_link_libraries(my_app PRIVATE bustache::bustache)
```

## Method 3: Using add_subdirectory

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_app)

# Add bustache as subdirectory
add_subdirectory(external/bustache)

# Create your executable
add_executable(my_app main.cpp)

# Link with bustache
target_link_libraries(my_app PRIVATE bustache::bustache)
```

## Example Code

```cpp
#include <bustache/render/string.hpp>
#include <iostream>

using namespace bustache;

int main() {
    // Create a template
    format tmpl("Hello {{name}}! You have {{count}} messages.");
    
    // Create data
    object data{
        {"name", "World"},
        {"count", 42}
    };
    
    // Render
    std::string result = to_string(tmpl(data));
    std::cout << result << std::endl;
    
    return 0;
}
```

## Configuration Options

When using bustache, you can configure these options:

```cmake
# Core options
option(BUSTACHE_BUILD_LIBRARY "Build as library (vs header-only)" ON)
option(BUSTACHE_USE_FMT "Use fmt library instead of std::format" OFF)
option(BUSTACHE_INSTALL "Generate installation target" ON)

# Development options
option(BUSTACHE_ENABLE_TESTING "Enable testing" OFF)
option(BUSTACHE_ENABLE_EXAMPLES "Build examples" OFF)
option(BUSTACHE_ENABLE_DOCS "Build documentation" OFF)
option(BUSTACHE_DEVELOPER_MODE "Enable warnings and sanitizers" OFF)
```

## Header-Only Mode

To use bustache as header-only:

```cmake
set(BUSTACHE_BUILD_LIBRARY OFF)
# Then link with bustache::headers instead of bustache::bustache
target_link_libraries(my_app PRIVATE bustache::headers)
```

## With fmt Library

If you prefer fmt over std::format:

```cmake
set(BUSTACHE_USE_FMT ON)
find_package(fmt REQUIRED)
# bustache will automatically link with fmt
```

## Advanced: Using Components

```cmake
find_package(bustache REQUIRED COMPONENTS headers)
# or
find_package(bustache REQUIRED COMPONENTS bustache)
```

## Troubleshooting

### Cannot find bustache
- Ensure bustache is installed or use FetchContent
- Set `CMAKE_PREFIX_PATH` to include bustache install directory
- Use `bustache_DIR` to point to the bustache config files

### C++20 Compiler Required
The refactored CMake automatically checks for C++20 support:
- GCC/Clang 10+
- MSVC 2019 16.10+
- AppleClang 12+

### Link Errors
- Ensure you're linking the correct target (bustache::bustache or bustache::headers)
- Check that BUSTACHE_BUILD_LIBRARY matches your usage