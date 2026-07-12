# ==============================================================================
# Makefile – Convenience wrapper around CMake for libcagekeygen.
#
# This Makefile provides simple targets for building, testing, installing,
# and cleaning the library without needing to remember the full CMake
# invocations.  It is intended for developers who prefer a `make` interface
# or for quick one‑off builds.
#
# All actual compilation is delegated to CMake; this file only calls CMake
# and the generated build system (e.g., Make or Ninja).  Therefore CMake
# ≥ 3.12 is still required.
#
# Usage:
#   make             – build the library in Release mode (default)
#   make BUILD_TYPE=Debug  – build in Debug mode
#   make test        – build and run the test suite (Debug mode)
#   make asan        – build and run tests with AddressSanitizer/UBSan
#   make install     – install the library and header system‑wide
#   make clean       – remove all build artifacts
#
# ------------------------------------------------------------------------------
# Variables you can override:
#   BUILD_TYPE       – CMake build type (Release, Debug, MinSizeRel, RelWithDebInfo)
#                      Default: Release
# ==============================================================================

# Disable built‑in suffix rules and implicit pattern rules for speed.
.PHONY: all clean build test asan install

# ------------------------------------------------------------------------------
# BUILD_TYPE : CMake build configuration.
#
# Valid values: Release, Debug, MinSizeRel, RelWithDebInfo.
# The default (Release) builds with optimisations and without debug symbols.
# Use BUILD_TYPE=Debug for development and testing.
# ------------------------------------------------------------------------------
BUILD_TYPE ?= Release

# ------------------------------------------------------------------------------
# all : Default target – build the library.
# ------------------------------------------------------------------------------
all: build

# ------------------------------------------------------------------------------
# build : Configure and compile the library.
#
# Creates a `build/` directory, runs CMake to generate the native build
# system, then invokes the build tool (e.g., make or ninja) to compile the
# library.  The build type is controlled by the BUILD_TYPE variable.
#
# Example:
#   make build BUILD_TYPE=Debug
# ------------------------------------------------------------------------------
build:
	cmake -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) .
	cmake --build build

# ------------------------------------------------------------------------------
# clean : Remove all build artifacts and test directories.
#
# Deletes the `build/`, `build_test/`, and (if present) `build_asan/`
# directories entirely.  This ensures a completely fresh start for the
# next build.
# ------------------------------------------------------------------------------
clean:
	rm -rf build build_test build_asan

# ------------------------------------------------------------------------------
# install : Build the library (if necessary) and install it.
#
# Requires that `build/` already contains a configured and compiled project.
# Calls `cmake --install build` which installs the library to the system
# directories (e.g., /usr/local/lib and /usr/local/include) as specified
# by the CMake installation rules.
#
# Tip: Set DESTDIR or CMAKE_INSTALL_PREFIX if you want to install to a
#      non‑standard location.
# ------------------------------------------------------------------------------
install: build
	cmake --install build

# ------------------------------------------------------------------------------
# test : Build the library and test executable in Debug mode, then run all tests.
#
# This target creates a separate build directory (`build_test/`) to avoid
# interfering with a release build.  It enables the BUILD_TESTING option,
# compiles everything, and runs the tests via CTest.  Test output is shown
# only on failure (`--output-on-failure`).
#
# The build type is forced to Debug so that assertions and debugging
# symbols are available during testing.
# ------------------------------------------------------------------------------
test:
	cmake -B build_test -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug .
	cmake --build build_test
	cd build_test && ctest --output-on-failure

# ------------------------------------------------------------------------------
# asan : Build and test with AddressSanitizer and UndefinedBehaviorSanitizer.
#
# Creates yet another separate build directory (`build_asan/`).  The
# ENABLE_SANITIZERS option is turned ON, which adds
# `-fsanitize=address,undefined` to both compile and link flags.  This
# helps catch memory errors and undefined behaviour.
#
# Like `test`, the build type is Debug to get full error reporting.
# ------------------------------------------------------------------------------
asan:
	cmake -B build_asan -DBUILD_TESTING=ON -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug .
	cmake --build build_asan
	cd build_asan && ctest --output-on-failure
