/**
 * @file test_utils.h
 * @brief Minimal unit‑test framework for the libcagekeygen test suite.
 *
 * This header provides a set of macros that simplify writing C unit tests
 * without depending on a full‑featured testing library.  The framework
 * tracks the total number of tests run and the number of failures through
 * two file‑scope counters:
 *
 * - `tests_run`    – incremented each time a test assertion is evaluated.
 * - `tests_failed` – incremented each time an assertion fails.
 *
 * Macros provided:
 * - `TEST(expr)`       – evaluate an arbitrary boolean expression.
 * - `TEST_EQ(a, b)`    – check that (a) == (b).
 * - `TEST_NEQ(a, b)`   – check that (a) != (b).
 * - `TEST_STREQ(a, b)` – check that strcmp(a, b) == 0.
 * - `PRINT_SUMMARY()`  – print the final counts to stdout.
 *
 * When an assertion fails, the macro prints the file name, line number,
 * and the failed expression to `stderr`.  The counters are defined as
 * `static` variables in the header; each translation unit that includes
 * this header gets its own copy.  The test runner in `test.c` is the only
 * file that includes this header, so the counters are effectively global
 * to the whole test program.
 *
 * Example usage:
 * @code
 *   #include "test_utils.h"
 *   void my_tests() {
 *       int x = 5;
 *       TEST_EQ(x, 5);
 *       TEST(x > 0);
 *   }
 *   int main() {
 *       my_tests();
 *       PRINT_SUMMARY();
 *       return tests_failed ? 1 : 0;
 *   }
 * @endcode
 *
 * @note This framework is intentionally minimal.  There is no test
 *       registration, suite hierarchy, or setup/teardown support.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Total number of test assertions executed so far.
 *
 * Incremented by the `TEST()` family of macros.  Used by `PRINT_SUMMARY()`
 * to display overall progress.  Because this variable is `static`, each
 * translation unit has its own copy.  In the current test setup only
 * `test.c` includes `test_utils.h`, so the counter is effectively
 * shared across all test suites.
 */
static int tests_run = 0;

/**
 * @brief Number of test assertions that have failed.
 *
 * Incremented whenever a `TEST()` condition evaluates to false.  At the
 * end of the test run, `main()` returns a non‑zero exit code if this
 * value is greater than zero.
 */
static int tests_failed = 0;

/**
 * @brief Core test macro – evaluate a boolean expression.
 *
 * Increments `tests_run`, then evaluates @p expr.  If the expression is
 * false (i.e., zero), the macro prints a failure message to `stderr`
 * containing the file name, line number, and the stringified expression,
 * and increments `tests_failed`.
 *
 * @param expr  Any scalar expression that can be evaluated in a boolean
 *              context (e.g., `x == 5`, `memcmp(a,b,len) == 0`).
 *
 * @note The macro uses a `do { ... } while(0)` wrapper so that it can be
 *       safely followed by a semicolon and used inside `if`/`else` blocks
 *       without braces.
 */
#define TEST(expr) do { \
    tests_run++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        tests_failed++; \
    } \
} while(0)

/**
 * @brief Assert that two values are equal using the `==` operator.
 *
 * Internally expands to `TEST((a) == (b))`.
 *
 * @param a  First value (any type compatible with `==`).
 * @param b  Second value.
 *
 * Example:
 * @code
 *   TEST_EQ(age_generate_keypair(pk, sk), AGE_OK);
 * @endcode
 */
#define TEST_EQ(a,b) TEST((a) == (b))

/**
 * @brief Assert that two values are not equal using the `!=` operator.
 *
 * Internally expands to `TEST((a) != (b))`.
 *
 * @param a  First value.
 * @param b  Second value.
 */
#define TEST_NEQ(a,b) TEST((a) != (b))

/**
 * @brief Assert that two C strings are equal (`strcmp` returns 0).
 *
 * Internally expands to `TEST(strcmp((a),(b)) == 0)`.
 *
 * @param a  First null‑terminated string.
 * @param b  Second null‑terminated string.
 *
 * @note Both arguments must be valid C strings (null‑terminated).
 *       Passing `NULL` will cause undefined behaviour (as with `strcmp`).
 */
#define TEST_STREQ(a,b) TEST(strcmp((a),(b)) == 0)

/**
 * @brief Print a summary of the tests executed so far.
 *
 * Outputs a line to `stdout` in the format:
 * @code
 *   Tests run: N, failed: M
 * @endcode
 * where `N` is the value of `tests_run` and `M` is the value of
 * `tests_failed` at the moment the macro is invoked.
 *
 * Typically called once at the end of `main()` before checking the exit
 * code.
 */
#define PRINT_SUMMARY() do { \
    printf("Tests run: %d, failed: %d\n", tests_run, tests_failed); \
} while(0)

#endif /* TEST_UTILS_H */
