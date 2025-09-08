#!/bin/bash
# =============================================================================
# Bustache - Build Script with Sanitizers
# =============================================================================
# This script provides easy local testing with various sanitizer configurations
# Usage: ./build_with_sanitizers.sh [compiler] [sanitizer] [build_type]
#
# Examples:
#   ./build_with_sanitizers.sh gcc address debug
#   ./build_with_sanitizers.sh clang undefined release
#   ./build_with_sanitizers.sh clang memory debug
#   ./build_with_sanitizers.sh msvc address debug
#
# =============================================================================

set -e  # Exit on any error

# Default values
COMPILER="${1:-gcc}"
SANITIZER="${2:-address}"
BUILD_TYPE="${3:-Debug}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to detect OS
detect_os() {
    case "$OSTYPE" in
        linux*)   echo "linux" ;;
        darwin*)  echo "macos" ;;
        msys*)    echo "windows" ;;
        cygwin*)  echo "windows" ;;
        *)        echo "unknown" ;;
    esac
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to get compiler executable
get_compiler_exe() {
    local compiler="$1"
    local os="$2"
    
    case "$compiler" in
        gcc)
            if command_exists gcc; then
                echo "gcc"
            elif command_exists gcc-12; then
                echo "gcc-12"
            elif command_exists gcc-11; then
                echo "gcc-11"
            else
                print_error "GCC not found"
                exit 1
            fi
            ;;
        clang)
            if command_exists clang++; then
                echo "clang++"
            elif command_exists clang++-17; then
                echo "clang++-17"
            elif command_exists clang++-16; then
                echo "clang++-16"
            elif command_exists clang++-15; then
                echo "clang++-15"
            elif command_exists clang++-14; then
                echo "clang++-14"
            else
                print_error "Clang not found"
                exit 1
            fi
            ;;
        apple-clang)
            if [ "$os" = "macos" ] && command_exists clang++; then
                echo "clang++"
            else
                print_error "Apple Clang only available on macOS"
                exit 1
            fi
            ;;
        msvc)
            if [ "$os" = "windows" ]; then
                echo "cl"
            else
                print_error "MSVC only available on Windows"
                exit 1
            fi
            ;;
        *)
            print_error "Unknown compiler: $compiler"
            exit 1
            ;;
    esac
}

# Function to validate sanitizer for compiler
validate_sanitizer() {
    local compiler="$1"
    local sanitizer="$2"
    
    case "$compiler" in
        gcc)
            case "$sanitizer" in
                address|undefined|thread|leak) return 0 ;;
                memory) print_error "Memory sanitizer not supported with GCC"; exit 1 ;;
                none) return 0 ;;
                *) print_error "Unknown sanitizer: $sanitizer"; exit 1 ;;
            esac
            ;;
        clang)
            case "$sanitizer" in
                address|undefined|thread|leak|memory) return 0 ;;
                none) return 0 ;;
                *) print_error "Unknown sanitizer: $sanitizer"; exit 1 ;;
            esac
            ;;
        apple-clang)
            case "$sanitizer" in
                address|undefined) return 0 ;;
                thread) print_warning "ThreadSanitizer may have issues on Apple Clang"; return 0 ;;
                memory) print_error "MemorySanitizer not supported with Apple Clang"; exit 1 ;;
                leak) print_warning "LeakSanitizer included with AddressSanitizer on Apple Clang"; return 0 ;;
                none) return 0 ;;
                *) print_error "Unknown sanitizer: $sanitizer"; exit 1 ;;
            esac
            ;;
        msvc)
            case "$sanitizer" in
                address) return 0 ;;
                none) return 0 ;;
                *) print_error "Only address sanitizer supported with MSVC"; exit 1 ;;
            esac
            ;;
    esac
}

# Function to set sanitizer environment variables
set_sanitizer_env() {
    local sanitizer="$1"
    local os="$2"
    
    case "$sanitizer" in
        address)
            # Disable leak detection on macOS as it's not supported
            if [ "$os" = "macos" ]; then
                export ASAN_OPTIONS="detect_leaks=0:abort_on_error=1:check_initialization_order=1:strict_init_order=1"
            else
                export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1:check_initialization_order=1:strict_init_order=1"
            fi
            print_info "Set ASAN_OPTIONS: $ASAN_OPTIONS"
            ;;
        undefined)
            export UBSAN_OPTIONS="print_stacktrace=1:abort_on_error=1"
            print_info "Set UBSAN_OPTIONS: $UBSAN_OPTIONS"
            ;;
        memory)
            export MSAN_OPTIONS="abort_on_error=1:print_stats=1"
            print_info "Set MSAN_OPTIONS: $MSAN_OPTIONS"
            ;;
        thread)
            export TSAN_OPTIONS="abort_on_error=1"
            print_info "Set TSAN_OPTIONS: $TSAN_OPTIONS"
            ;;
        leak)
            export LSAN_OPTIONS="abort_on_error=1"
            print_info "Set LSAN_OPTIONS: $LSAN_OPTIONS"
            ;;
    esac
}

# Function to build with CMake
build_with_cmake() {
    local compiler_exe="$1"
    local sanitizer="$2"
    local build_type="$3"
    local build_dir="$4"
    
    print_info "Configuring CMake..."
    
    # Base CMake arguments
    local cmake_args=(
        "-S" "."
        "-B" "$build_dir"
        "-DCMAKE_BUILD_TYPE=$build_type"
        "-DBUSTACHE_ENABLE_TESTING=ON"
        "-DBUSTACHE_DEVELOPER_MODE=ON"
    )
    
    # Add compiler if not MSVC
    if [ "$COMPILER" != "msvc" ]; then
        cmake_args+=("-DCMAKE_CXX_COMPILER=$compiler_exe")
    fi
    
    # Note: Apple Clang is automatically detected by CMake when using clang++ on macOS
    
    # Add generator
    if command_exists ninja; then
        cmake_args+=("-G" "Ninja")
    fi
    
    # Add sanitizer flags
    case "$sanitizer" in
        address)
            cmake_args+=("-DBUSTACHE_ENABLE_SANITIZER_ADDRESS=ON")
            ;;
        undefined)
            cmake_args+=("-DBUSTACHE_ENABLE_SANITIZER_UNDEFINED=ON")
            ;;
        memory)
            cmake_args+=("-DBUSTACHE_ENABLE_SANITIZER_MEMORY=ON")
            ;;
        thread)
            cmake_args+=("-DBUSTACHE_ENABLE_SANITIZER_THREAD=ON")
            ;;
        leak)
            cmake_args+=("-DBUSTACHE_ENABLE_SANITIZER_LEAK=ON")
            ;;
    esac
    
    # Run CMake configure
    cmake "${cmake_args[@]}"
    
    print_info "Building..."
    cmake --build "$build_dir" --parallel
    
    print_success "Build completed successfully"
}

# Function to run tests
run_tests() {
    local build_dir="$1"
    
    print_info "Running tests..."
    cd "$build_dir"
    
    if command_exists ctest; then
        ctest --output-on-failure --parallel --timeout 300
        local test_result=$?
        
        if [ $test_result -eq 0 ]; then
            print_success "All tests passed!"
        else
            print_error "Some tests failed (exit code: $test_result)"
            return $test_result
        fi
    else
        print_error "ctest not found"
        return 1
    fi
}

# Main execution
main() {
    print_info "Bustache Build Script with Sanitizers"
    print_info "======================================"
    print_info "Compiler: $COMPILER"
    print_info "Sanitizer: $SANITIZER"
    print_info "Build Type: $BUILD_TYPE"
    print_info ""
    
    # Detect OS
    OS=$(detect_os)
    print_info "Detected OS: $OS"
    
    # Validate inputs
    validate_sanitizer "$COMPILER" "$SANITIZER"
    
    # Get compiler executable
    COMPILER_EXE=$(get_compiler_exe "$COMPILER" "$OS")
    print_info "Using compiler: $COMPILER_EXE"
    
    # Set sanitizer environment
    if [ "$SANITIZER" != "none" ]; then
        set_sanitizer_env "$SANITIZER" "$OS"
    fi
    
    # Create build directory
    BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
    BUILD_DIR="build_${COMPILER}_${SANITIZER}_${BUILD_TYPE_LOWER}"
    print_info "Build directory: $BUILD_DIR"
    
    # Clean build directory if it exists
    if [ -d "$BUILD_DIR" ]; then
        print_warning "Removing existing build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
    
    # Build
    build_with_cmake "$COMPILER_EXE" "$SANITIZER" "$BUILD_TYPE" "$BUILD_DIR"
    
    # Run tests
    run_tests "$BUILD_DIR"
    
    print_success "Build and test completed successfully!"
    print_info "Build artifacts are in: $BUILD_DIR"
}

# Help function
show_help() {
    cat << EOF
Bustache Build Script with Sanitizers

Usage: $0 [compiler] [sanitizer] [build_type]

Arguments:
  compiler    Compiler to use (gcc, clang, apple-clang, msvc) [default: gcc]
  sanitizer   Sanitizer to enable (address, undefined, memory, thread, leak, none) [default: address]
  build_type  Build type (Debug, Release) [default: Debug]

Examples:
  $0                             # GCC with AddressSanitizer in Debug
  $0 gcc address debug           # Same as above (explicit)
  $0 clang undefined release     # Clang with UBSan in Release
  $0 clang memory debug          # Clang with MemorySanitizer in Debug
  $0 apple-clang address debug   # Apple Clang with AddressSanitizer in Debug
  $0 msvc address debug          # MSVC with AddressSanitizer in Debug

Supported sanitizer combinations:
  GCC:         address, undefined, thread, leak
  Clang:       address, undefined, memory, thread, leak  
  Apple Clang: address, undefined, (thread)
  MSVC:        address

Environment variables:
  The script automatically sets appropriate sanitizer environment variables:
  - ASAN_OPTIONS for AddressSanitizer
  - UBSAN_OPTIONS for UndefinedBehaviorSanitizer
  - MSAN_OPTIONS for MemorySanitizer
  - TSAN_OPTIONS for ThreadSanitizer
  - LSAN_OPTIONS for LeakSanitizer

Requirements:
  - CMake 3.15+
  - C++20 compatible compiler
  - Ninja (optional, for faster builds)
  - Catch2 (automatically fetched if not found)

EOF
}

# Check for help
if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    show_help
    exit 0
fi

# Run main function
main "$@"
