/**
 * @file test_gen.c
 * @brief Unit tests for key generation and derivation functions of
 *        libcagekeygen.
 *
 * This module exercises `age_generate_keypair()` and
 * `age_public_key_from_secret_key()`. The tests verify:
 *
 * - Basic generation produces non‑zero secret and public keys.
 * - Deriving a public key from a secret key yields a result consistent
 *   with the original key pair.
 * - Successive calls to `age_generate_keypair()` return different keys
 *   (i.e., the generator is not deterministic and the random source is
 *   working).
 * - An all‑zero secret key is correctly rejected by
 *   `age_public_key_from_secret_key()` with `AGE_ERR_KEYGEN_FAILED`.
 *
 * The helper `is_zero()` provides a simple constant‑time‑ish check for an
 * all‑zero buffer (used to confirm keys are not zero after generation).
 *
 * The tests rely on the minimal framework defined in `test_utils.h`:
 * `TEST_EQ()` for equality checks and `TEST()` for arbitrary boolean
 * expressions.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 */

#include "libcagekeygen.h"
#include "test_utils.h"
#include <string.h>

/**
 * @brief Check if a byte buffer is entirely zero.
 *
 * Used to ensure that freshly generated keys are not trivially empty.
 *
 * @param[in] buf  Pointer to the buffer to check.
 * @param[in] len  Number of bytes to examine.
 *
 * @return 1 if all bytes are zero, 0 otherwise.
 */
static int is_zero(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++)
    if (buf[i] != 0)
      return 0;
  return 1;
}

/**
 * @brief Run all generation and derivation tests.
 *
 * This function is called by the test runner in `test.c`. It groups four
 * test cases:
 *
 * 1. **Basic generation**
 *    Calls `age_generate_keypair()` and asserts success. Then verifies that
 *    both the public and secret keys are non‑zero.
 *
 * 2. **Derivation consistency**
 *    Derives the public key from the generated secret key using
 *    `age_public_key_from_secret_key()` and compares it with the original
 *    public key. They must be equal.
 *
 * 3. **Randomness between generations**
 *    Generates a second key pair and checks that both the secret key and
 *    the public key differ from the first pair. This confirms that the
 *    random source is providing fresh entropy on each call.
 *
 * 4. **Weak key rejection**
 *    Passes an all‑zero secret key to `age_public_key_from_secret_key()`.
 *    The function must return `AGE_ERR_KEYGEN_FAILED` because the resulting
 *    public key would be all‑zero, which is forbidden.
 */
void test_gen(void) {
  uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];

  /* ---------------------------------------------------------------
   * 1. Basic generation
   * --------------------------------------------------------------- */
  TEST_EQ(age_generate_keypair(pk, sk), AGE_OK);
  /* Keys must not be all‑zero. */
  TEST(!is_zero(pk, AGE_KEY_BYTES));
  TEST(!is_zero(sk, AGE_KEY_BYTES));

  /* ---------------------------------------------------------------
   * 2. Derivation consistency
   * --------------------------------------------------------------- */
  uint8_t pk2[AGE_KEY_BYTES];
  TEST_EQ(age_public_key_from_secret_key(sk, pk2), AGE_OK);
  /* Derived public key must match the original. */
  TEST(memcmp(pk, pk2, AGE_KEY_BYTES) == 0);

  /* ---------------------------------------------------------------
   * 3. Two successive generations must differ
   * --------------------------------------------------------------- */
  uint8_t pk3[AGE_KEY_BYTES], sk3[AGE_KEY_BYTES];
  TEST_EQ(age_generate_keypair(pk3, sk3), AGE_OK);
  /* Secret keys should be different (overwhelming probability). */
  TEST(memcmp(sk, sk3, AGE_KEY_BYTES) != 0);
  /* Public keys should also be different. */
  TEST(memcmp(pk, pk3, AGE_KEY_BYTES) != 0);

  /* ---------------------------------------------------------------
   * 4. All‑zero secret key must be rejected
   * --------------------------------------------------------------- */
  uint8_t zeros[AGE_KEY_BYTES] = {0};
  TEST_EQ(age_public_key_from_secret_key(zeros, pk), AGE_ERR_KEYGEN_FAILED);
}
