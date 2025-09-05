# Bustache CI/CD and Testing Guide

This document describes the comprehensive CI/CD setup and testing infrastructure for the Bustache project.

## Overview

Bustache uses a multi-platform, multi-compiler CI/CD pipeline that ensures compatibility and reliability across different environments. The CI system tests:

- **4 Compiler Families**: GCC, LLVM Clang, Apple Clang, MSVC
- **Multiple Versions**: GCC 10-13, Clang 13-17, Apple Clang (Xcode 14.2-15.4), MSVC 2019/2022
- **2 Build Types**: Debug and Release
- **5 Sanitizers**: AddressSanitizer, UBSan, MemorySanitizer, ThreadSanitizer, LeakSanitizer
- **Special Configurations**: Header-only mode, fmt library integration, coverage analysis

## CI Pipeline Structure

### 1. Linux GCC Builds (`linux-gcc`)
- **Compilers**: GCC 10, 11, 12, 13
- **Build Types**: Debug, Release
- **Sanitizers**: AddressSanitizer, UBSan, ThreadSanitizer
- **Platform**: Ubuntu 22.04
- **Features**:
  - Comprehensive sanitizer coverage
  - Memory leak detection
  - Undefined behavior detection
  - Thread safety validation

### 2. Linux LLVM Clang Builds (`linux-clang`)
- **Compilers**: Clang 13, 14, 15, 16, 17
- **Build Types**: Debug, Release
- **Sanitizers**: AddressSanitizer, UBSan, MemorySanitizer, ThreadSanitizer
- **Platform**: Ubuntu 22.04
- **Features**:
  - MemorySanitizer (Clang-exclusive)
  - Advanced memory error detection
  - Cross-compiler compatibility validation

### 3. macOS Apple Clang Builds (`macos-apple-clang`)
- **Compilers**: Apple Clang (Xcode 14.2, 15.2, 15.4)
- **Build Types**: Debug, Release
- **Sanitizers**: AddressSanitizer, UBSan
- **Platforms**: macOS 12 (Monterey), macOS 13 (Ventura), macOS 14 (Sonoma)
- **Features**:
  - macOS-specific compatibility
  - Apple Clang optimizations
  - Cross-platform validation

### 4. Windows MSVC Builds (`windows-msvc`)
- **Compilers**: Visual Studio 2019, 2022
- **Build Types**: Debug, Release
- **Sanitizers**: AddressSanitizer (where supported)
- **Platforms**: Windows Server 2019, 2022
- **Features**:
  - Windows-specific compatibility
  - MSVC-specific optimizations
  - Cross-platform validation

### 5. Special Configurations (`special-configs`)
- **Header-only Mode**: Tests library usage without compilation
- **fmt Library Integration**: Tests with external fmt library
- **Coverage Analysis**: Code coverage reporting with lcov

### 6. Documentation and Examples (`docs-and-examples`)
- **Documentation**: Doxygen-generated API docs
- **Examples**: Validation of example code
- **Integration Testing**: Real-world usage scenarios

## Sanitizer Configuration

### AddressSanitizer (ASan)
- **Purpose**: Detects buffer overflows, use-after-free, memory leaks
- **Supported**: GCC, Clang, MSVC
- **Environment**: `ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:check_initialization_order=1:strict_init_order=1`

### UndefinedBehaviorSanitizer (UBSan)
- **Purpose**: Detects undefined behavior (integer overflow, null pointer dereference, etc.)
- **Supported**: GCC, Clang
- **Environment**: `UBSAN_OPTIONS=print_stacktrace=1:abort_on_error=1`

### MemorySanitizer (MSan)
- **Purpose**: Detects uninitialized memory reads
- **Supported**: Clang only (15+)
- **Environment**: `MSAN_OPTIONS=abort_on_error=1:print_stats=1`

### ThreadSanitizer (TSan)
- **Purpose**: Detects data races and thread safety issues
- **Supported**: GCC, Clang
- **Environment**: `TSAN_OPTIONS=abort_on_error=1`
- **Note**: Cannot be combined with ASan/LSan

### LeakSanitizer (LSan)
- **Purpose**: Detects memory leaks
- **Supported**: GCC, Clang
- **Environment**: `LSAN_OPTIONS=abort_on_error=1`
- **Note**: Often enabled automatically with ASan

## Local Testing

### Using the Build Script

The project includes a comprehensive build script for local testing with sanitizers:

```bash
# Basic usage
./build_with_sanitizers.sh [compiler] [sanitizer] [build_type]

# Examples
./build_with_sanitizers.sh gcc address debug      # GCC with AddressSanitizer
./build_with_sanitizers.sh clang undefined release # Clang with UBSan
./build_with_sanitizers.sh clang memory debug     # Clang with MemorySanitizer
```

### Manual CMake Configuration

For custom configurations:

```bash
# GCC with AddressSanitizer
cmake -S . -B build_gcc_asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=g++ \
  -DBUSTACHE_ENABLE_TESTING=ON \
  -DBUSTACHE_DEVELOPER_MODE=ON \
  -DBUSTACHE_ENABLE_SANITIZER_ADDRESS=ON

# Clang with MemorySanitizer
cmake -S . -B build_clang_msan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DBUSTACHE_ENABLE_TESTING=ON \
  -DBUSTACHE_DEVELOPER_MODE=ON \
  -DBUSTACHE_ENABLE_SANITIZER_MEMORY=ON

# Build and test
cmake --build build_gcc_asan --parallel
cd build_gcc_asan && ctest --output-on-failure
```

## Test Suite Overview

The project includes 12 comprehensive test categories:

### Core Functionality Tests
1. **`specs`**: Mustache specification compliance tests
2. **`unresolved_handler`**: Variable resolution behavior
3. **`udt`**: User-defined type integration
4. **`inheritance`**: Template inheritance extension
5. **`split_tag`**: Tag parsing and processing

### Advanced Feature Tests
6. **`dynamic_names`**: Dynamic partial name resolution
7. **`context_stack`**: Context management and scoping
8. **`dynamic_partials_tests`**: Dynamic partial loading
9. **`custom_extensions_tests`**: Custom extension mechanisms

### Robustness Tests
10. **`boundary_tests`**: Edge cases and boundary conditions
11. **`error_path_tests`**: Error handling and recovery
12. **`failure_simulation_tests`**: Failure mode testing

### Performance Tests
13. **`performance_tests`**: Performance benchmarking and optimization validation

## Compiler Support Matrix

| Compiler     | Version | C++20 | ASan | UBSan | MSan | TSan | LSan |
|--------------|---------|--------|------|-------|------|------|------|
| GCC          | 10+     | ✅     | ✅   | ✅    | ❌   | ✅   | ✅   |
| Clang        | 13+     | ✅     | ✅   | ✅    | ✅   | ✅   | ✅   |
| Apple Clang  | 14.2+   | ✅     | ✅   | ✅    | ❌   | ⚠️   | ✅   |
| MSVC         | 19.29+  | ✅     | ✅   | ❌    | ❌   | ❌   | ❌   |

## Platform Support Matrix

| Platform | GCC | Clang | Apple Clang | MSVC | Sanitizers | Notes |
|----------|-----|-------|-------------|------|------------|--------|
| Linux    | ✅  | ✅    | ❌         | ❌   | All        | Primary development platform |
| Windows  | ⚠️  | ⚠️    | ❌         | ✅   | ASan only  | MSVC recommended |
| macOS    | ⚠️  | ⚠️    | ✅         | ❌   | ASan, UBSan| Apple Clang recommended |

Legend: ✅ Full support, ⚠️ Limited support, ❌ Not supported

## Coverage Reporting

Code coverage is generated using:
- **Tool**: lcov (GCC-based)
- **Upload**: Codecov integration
- **Exclusions**: System headers, test files, dependencies
- **Threshold**: Configurable per project needs

## Best Practices

### For Contributors

1. **Always test with sanitizers** before submitting PRs
2. **Use the build script** for consistent local testing
3. **Check CI status** on all platforms before merging
4. **Add tests** for new features and bug fixes
5. **Update documentation** for API changes

### For Maintainers

1. **Monitor CI performance** and adjust timeouts as needed
2. **Update compiler versions** regularly
3. **Review sanitizer findings** carefully
4. **Maintain test coverage** above acceptable thresholds
5. **Document breaking changes** in CI configuration

## Troubleshooting

### Common Issues

1. **Sanitizer false positives**: Check sanitizer suppression files
2. **Timeout failures**: Increase test timeout or optimize slow tests
3. **Compiler compatibility**: Verify C++20 feature usage
4. **Platform-specific failures**: Check platform-specific code paths

### Debug Commands

```bash
# Verbose test output
ctest --output-on-failure --verbose

# Run specific test
ctest -R test_name --output-on-failure

# Debug sanitizer issues
export ASAN_OPTIONS="abort_on_error=0:halt_on_error=1"
./test_executable

# Check compiler version
gcc --version
clang++ --version
```

## Future Enhancements

### Planned Additions
- [ ] Apple Clang support (macOS CI)
- [ ] ARM64 testing (native and cross-compilation)
- [ ] Static analysis integration (clang-static-analyzer, PVS-Studio)
- [ ] Fuzzing integration (libFuzzer, AFL++)
- [ ] Performance regression detection
- [ ] Benchmark comparison across commits

### Potential Improvements
- [ ] Matrix build optimization for faster CI
- [ ] Parallel test execution improvements
- [ ] Custom sanitizer configurations
- [ ] Integration with package managers (Conan, vcpkg)
- [ ] Docker-based testing environments

## Contributing to CI

When contributing to the CI configuration:

1. **Test changes locally** using multiple configurations
2. **Document any new features** in this README
3. **Consider CI time impact** of new jobs
4. **Maintain backward compatibility** where possible
5. **Follow the existing naming conventions**

For questions about CI configuration, please open an issue with the `ci` label.
