# -----------------------------------------------------------------------------
# CMakeLists.txt — Test suite for the libcagekeygen library.
#
# This file builds the test runner executable and registers it with CTest.
# It is included from the project root when BUILD_TESTING is enabled.
#
# Usage:
#   cmake -B build_test -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug .
#   cmake --build build_test
#   ctest --test-dir build_test --output-on-failure
#
# The test executable links against the cagekeygen library and runs all
# unit tests defined in the test/*.c files.
# -----------------------------------------------------------------------------

# Build the test executable from the listed sources.
# test.c          – main() entry point; calls individual test suites.
# test_gen.c      – tests for key generation and derivation.
# test_io.c       – tests for string encoding/decoding.
# test_random.c   – randomised round‑trip and consistency checks (500 iterations).
add_executable(test_libcagekeygen
    test.c
    test_gen.c
    test_io.c
    test_random.c
)

# Link against the static (or shared) cagekeygen library.
# The library target must be defined in the root CMakeLists.txt.
target_link_libraries(test_libcagekeygen PRIVATE cagekeygen)

# Add the include directory so test files can include "libcagekeygen.h"
# and the test utility header "test_utils.h".
target_include_directories(test_libcagekeygen PRIVATE ../include)

# -----------------------------------------------------------------------------
# Sanitizer support (optional)
# When ENABLE_SANITIZERS is ON at the root level, apply AddressSanitizer (ASan)
# and UndefinedBehaviorSanitizer (UBSan) to the test executable as well.
# The -fno-omit-frame-pointer flag improves stack traces in error reports.
# -----------------------------------------------------------------------------
if(ENABLE_SANITIZERS)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(test_libcagekeygen PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(test_libcagekeygen PRIVATE
            -fsanitize=address,undefined
        )
    endif()
endif()

# -----------------------------------------------------------------------------
# Strict compiler warnings
# Enable a high level of warnings and treat them as errors to catch potential
# issues early. The flags are the same as those used for the library itself.
# -----------------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_libcagekeygen PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# Register the test with CTest so that `ctest` can discover and run it.
# The test is named "test_libcagekeygen" and the command is simply the
# executable name (CMake adds the build path automatically).
add_test(NAME test_libcagekeygen COMMAND test_libcagekeygen)
