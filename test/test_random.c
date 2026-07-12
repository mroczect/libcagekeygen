/**
 * @file test_random.c
 * @brief Randomised end‑to‑end (round‑trip) tests for libcagekeygen.
 *
 * This module executes a large number of randomised iterations that
 * exercise every public function of the library in sequence:
 *
 * 1. Generate a fresh key pair with `age_generate_keypair()`.
 * 2. Encode the public key to a string with `age_public_key_to_string()`.
 * 3. Decode the public key string back with `age_string_to_public_key()` and
 *    verify that the raw key matches the original.
 * 4. Encode the secret key to a string with `age_secret_key_to_string()`.
 * 5. Decode the secret key string back with `age_string_to_secret_key()` and
 *    verify the raw key matches.
 * 6. Derive the public key from the decoded secret key using
 *    `age_public_key_from_secret_key()` and confirm it is identical to the
 *    original public key.
 *
 * The loop runs **500 times**, each iteration with a different random key.
 * This provides strong confidence that the encoding/decoding and derivation
 * paths are consistent, and that no edge cases (e.g., near‑zero keys,
 * specific bit patterns in the Bech32 conversion) cause failures.
 *
 * If any test fails, the global failure counter is incremented by the
 * `TEST_EQ()` / `TEST()` macros from `test_utils.h`, and the error is
 * reported to stderr with file/line information. After all iterations,
 * the test runner in `test.c` prints a summary and exits with a non‑zero
 * code if any failures occurred.
 *
 * @note This test is **not** a cryptographic validation of the X25519
 *       implementation; it only checks software correctness (round‑trip
 *       stability).
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 */

#include "libcagekeygen.h"
#include "test_utils.h"
#include <string.h>

/**
 * @brief Perform 500 randomised round‑trip tests on all library functions.
 *
 * This function is called from the main test runner in `test.c`. It is
 * intentionally named `test_random_invalid` to reflect that it uses
 * random inputs (not that the inputs are invalid) and to distinguish it
 * from the deterministic tests in `test_io.c` and `test_gen.c`.
 *
 * Each iteration:
 * - Generates a key pair and checks for `AGE_OK`.
 * - Performs public key string round‑trip and asserts byte‑level equality.
 * - Performs secret key string round‑trip and asserts byte‑level equality.
 * - Derives the public key from the decoded secret and compares it to the
 *   original public key.
 *
 * The test harness macros (`TEST_EQ`, `TEST`) will capture any failure
 * and print diagnostic information automatically.
 */
void test_random_invalid(void) {
  // Randomised roundtrip
  for (int i = 0; i < 500; i++) {
    uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
    TEST_EQ(age_generate_keypair(pk, sk), AGE_OK);

    char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
    TEST_EQ(age_public_key_to_string(pk, pub_str), AGE_OK);
    uint8_t pk2[AGE_KEY_BYTES];
    TEST_EQ(age_string_to_public_key(pub_str, pk2), AGE_OK);
    TEST(memcmp(pk, pk2, AGE_KEY_BYTES) == 0);

    char sec_str[AGE_SECRET_KEY_BUF_SIZE];
    TEST_EQ(age_secret_key_to_string(sk, sec_str), AGE_OK);
    uint8_t sk2[AGE_KEY_BYTES];
    TEST_EQ(age_string_to_secret_key(sec_str, sk2), AGE_OK);
    TEST(memcmp(sk, sk2, AGE_KEY_BYTES) == 0);

    // Derive from decoded secret
    uint8_t pk3[AGE_KEY_BYTES];
    TEST_EQ(age_public_key_from_secret_key(sk2, pk3), AGE_OK);
    TEST(memcmp(pk, pk3, AGE_KEY_BYTES) == 0);
  }
}
