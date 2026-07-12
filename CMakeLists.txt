# ==============================================================================
# CMakeLists.txt – Root build configuration for libcagekeygen.
#
# libcagekeygen is a small C library that generates and handles X25519 key
# pairs in the format used by the age encryption tool.  This file defines
# the library target, sets up compiler flags, optional features (sanitizers,
# testing), and installation rules.
#
# Minimum CMake version required: 3.12
# Supported languages:            C
# C standard:                     C11 (required)
#
# Build options (can be set via -D on the command line):
#   BUILD_SHARED_LIBS  – Build as a shared library instead of static (OFF by default).
#   BUILD_TESTING      – Build the test suite and enable CTest (OFF by default).
#   ENABLE_SANITIZERS  – Compile with AddressSanitizer and UBSan (OFF by default).
#
# Usage examples:
#   cmake -B build -DCMAKE_BUILD_TYPE=Release .
#   cmake -B build_test -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug .
#   cmake -B build_asan -DBUILD_TESTING=ON -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug .
#   cmake --build build
#   ctest --test-dir build_test --output-on-failure
# ==============================================================================

# ------------------------------------------------------------------------------
# 1. Project definition
# ------------------------------------------------------------------------------
# VERSION 0.0.1 is the library version.  LANGUAGES C tells CMake that we only
# compile C code, which skips unnecessary compiler checks.
cmake_minimum_required(VERSION 3.12)
project(libcagekeygen VERSION 0.0.1 LANGUAGES C)

# ------------------------------------------------------------------------------
# 2. C standard
# ------------------------------------------------------------------------------
# We require C11 because the code uses features like anonymous unions?
# Actually the code uses C99 static array parameters, which are fine.
# C11 is safe and widely available.  The standard is REQUIRED, so CMake will
# fail if the compiler cannot provide it.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# 3. Build options (user-configurable)
# ------------------------------------------------------------------------------
# BUILD_SHARED_LIBS: When ON, the library will be built as a shared object
#   (.so/.dylib/.dll).  When OFF (the default), a static archive is produced.
#   The variable name is CMake's built-in; we just offer it as a documented
#   option.
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# BUILD_TESTING: Enables the test subdirectory and registers tests with CTest.
#   When ON, a separate `test_libcagekeygen` executable is built and linked
#   against the library.
option(BUILD_TESTING "Build tests" OFF)

# ENABLE_SANITIZERS: Compile both the library and (if enabled) the test
#   executable with AddressSanitizer (ASan) and UndefinedBehaviorSanitizer
#   (UBSan).  This option is only effective for GCC and Clang.
#   The flag is applied with -fsanitize=address,undefined.
option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior sanitizers" OFF)

# ------------------------------------------------------------------------------
# 4. Library target
# ------------------------------------------------------------------------------
# The library target is named "cagekeygen".  We list all source files and the
# public header (the header is included so that IDE projects display it
# conveniently; it is not compiled).
add_library(cagekeygen
    src/curve25519-donna.c   # External X25519 implementation (Donna)
    src/gen.c                # Key generation and derivation
    src/io.c                 # Bech32 encoding/decoding for keys
    src/error.c              # Error string conversion
    src/lib.c                # Unity-build aggregator (can be omitted)
    include/libcagekeygen.h  # Public API header
)

# The include directory is made PUBLIC so that consumers (test executables,
# external projects) can #include "libcagekeygen.h" without specifying the
# path manually.
target_include_directories(cagekeygen PUBLIC include)

# ------------------------------------------------------------------------------
# 5. Platform-specific dependencies
# ------------------------------------------------------------------------------
# Windows: The library uses BCryptGenRandom for secure randomness.  The
#          bcrypt library must be linked.
if(WIN32)
    target_link_libraries(cagekeygen PUBLIC bcrypt)
endif()
# Linux: getrandom() is provided by the C library; no extra linking needed.
# BSD/macOS: /dev/urandom is used; no extra library needed.

# ------------------------------------------------------------------------------
# 6. Compiler warnings (strict)
# ------------------------------------------------------------------------------
# For GCC and Clang, enable a high warning level and treat warnings as errors.
# This helps catch potential bugs early in development.  The flags are:
#   -Wall      – enable most common warnings
#   -Wextra    – additional warnings
#   -Wpedantic – enforce ISO C compliance
#   -Werror    – turn warnings into errors
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cagekeygen PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# ------------------------------------------------------------------------------
# 7. Sanitizers (optional)
# ------------------------------------------------------------------------------
# When ENABLE_SANITIZERS is ON and the compiler is GCC or Clang, we add the
# necessary compile and link flags for AddressSanitizer and UndefinedBehaviorSanitizer.
# -fno-omit-frame-pointer improves stack traces in sanitizer reports.
if(ENABLE_SANITIZERS)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(cagekeygen PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(cagekeygen PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# ------------------------------------------------------------------------------
# 8. Installation rules
# ------------------------------------------------------------------------------
# The GNUInstallDirs module sets standard installation directories (lib, bin,
# include, etc.) according to the platform's conventions.  We install:
#   - The library (static or shared) into ${CMAKE_INSTALL_LIBDIR} (e.g., /usr/lib)
#   - The public header into ${CMAKE_INSTALL_INCLUDEDIR} (e.g., /usr/include)
include(GNUInstallDirs)
install(TARGETS cagekeygen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # shared libraries
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # static libraries
)
install(FILES include/libcagekeygen.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ------------------------------------------------------------------------------
# 9. Testing (conditional)
# ------------------------------------------------------------------------------
# If BUILD_TESTING is ON, we enable CTest (which provides the `ctest` command)
# and add the test/ subdirectory, which contains its own CMakeLists.txt.
# That subdirectory builds the test executable and registers it as a CTest test.
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
