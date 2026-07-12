/**
 * @file test.c
 * @brief Test runner entry point for the libcagekeygen test suite.
 *
 * This file contains the `main()` function that orchestrates all unit tests
 * for the library. It calls the individual test suite functions defined in
 * separate test modules (`test_gen.c`, `test_io.c`, `test_random.c`), then
 * prints a summary of passed/failed tests and returns an appropriate exit
 * code.
 *
 * The tests are written using a minimal framework provided by
 * `test_utils.h`. The framework tracks global counters `tests_run` and
 * `tests_failed`, which are updated by the `TEST()` family of macros.
 * At the end of execution, `PRINT_SUMMARY()` displays the totals and the
 * return value of `main()` is 0 if all tests passed, 1 otherwise.
 *
 * This runner is compiled as part of the `test_libcagekeygen` executable
 * by the CMake build system (see `test/CMakeLists.txt`). Running the
 * executable directly or via `ctest` will execute all tests.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 *
 * @see test_utils.h for the test macros and infrastructure.
 */

#include "test_utils.h"

/* -----------------------------------------------------------------------
 * Forward declarations of test suites defined in other translation units.
 *
 * Each function contains a set of test cases for a specific area of the
 * library. They are declared here so that `main()` can call them.
 * ----------------------------------------------------------------------- */

/**
 * @brief Run all tests related to key generation and derivation.
 *
 * Defined in `test_gen.c`. Checks basic generation, consistency of derived
 * public keys, randomness between successive calls, and rejection of
 * degenerate inputs.
 */
void test_gen(void);

/**
 * @brief Run all tests for string encoding and decoding (I/O).
 *
 * Defined in `test_io.c`. Validates round‑tripping of public and secret key
 * strings, handling of invalid formats, and detection of weak keys.
 */
void test_io(void);

/**
 * @brief Run randomised round‑trip and consistency tests.
 *
 * Defined in `test_random.c`. Performs 500 iterations of key generation,
 * string conversion, and derivation to catch sporadic issues or edge cases.
 */
void test_random_invalid(void);

/**
 * @brief Main entry point for the test executable.
 *
 * Executes all test suites in the following order:
 * 1. Key generation and derivation (`test_gen()`)
 * 2. String encoding/decoding (`test_io()`)
 * 3. Randomised round‑trip checks (`test_random_invalid()`)
 *
 * After all suites have run, a summary of the total tests executed and
 * the number of failures is printed to standard output. The function
 * returns 0 if all tests passed, or 1 if at least one failure occurred.
 *
 * @return EXIT_SUCCESS (0) if all tests passed, EXIT_FAILURE (1) otherwise.
 */
int main(void) {
  test_gen();
  test_io();
  test_random_invalid();
  PRINT_SUMMARY();
  return tests_failed ? 1 : 0;
}
