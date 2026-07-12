/**
 * @file test_io.c
 * @brief Unit tests for the string encoding/decoding (I/O) functions of
 *        libcagekeygen.
 *
 * This module tests the four public API functions that convert between raw
 * 32‑byte keys and the standard age string representations:
 * - `age_public_key_to_string()`
 * - `age_secret_key_to_string()`
 * - `age_string_to_public_key()`
 * - `age_string_to_secret_key()`
 *
 * The tests cover:
 * - Successful round‑trip (encode → decode) for both key types.
 * - Verification of string length and prefix.
 * - Rejection of invalid inputs: wrong length, incorrect prefix, illegal
 *   characters, and weak (all‑zero) public keys.
 *
 * All tests use the lightweight framework provided by `test_utils.h`, which
 * relies on the `TEST()`, `TEST_EQ()`, and `TEST_STREQ()` macros to
 * accumulate pass/fail counts globally.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 */

#include "libcagekeygen.h"
#include "test_utils.h"
#include <string.h>

/**
 * @brief Execute all I/O‑related tests.
 *
 * Generates a fresh key pair with `age_generate_keypair()` and then
 * exercises the encoding/decoding functions under correct and incorrect
 * conditions. The function is called from the main test runner in
 * `test.c`.
 *
 * Test structure:
 * 1. Public key encoding to string:
 *    - Check that `age_public_key_to_string()` returns `AGE_OK`.
 *    - The output string has the expected length (56 characters).
 *    - The string starts with the `"age1"` prefix.
 * 2. Public key decoding from string:
 *    - A round‑trip: encode then decode, verify the raw keys match.
 * 3. Invalid public key strings:
 *    - String too short (only the prefix).
 *    - Wrong prefix (`"Xge1..."`).
 *    - Invalid Bech32 character (e.g., `"age1xxxx..."` where 'x' is not in the
 *      charset).
 * 4. Weak public key rejection:
 *    - Encode an all‑zero public key and attempt to decode it; the function
 *      must return `AGE_ERR_WEAK_PUBLIC_KEY`.
 * 5. Secret key encoding to string:
 *    - Check `AGE_OK` return.
 *    - Correct length (67 characters).
 *    - Starts with `"AGE-SECRET-KEY-1"`.
 * 6. Secret key round‑trip:
 *    - Encode then decode; raw keys must be identical.
 * 7. Invalid secret key strings:
 *    - Too short.
 *    - Wrong prefix (`"AGE-SECRET-KEY-2..."`).
 *    - Invalid character.
 *
 * @note No test for all‑zero secret key decoding is performed because the
 *       library intentionally accepts any validly‑formatted secret key
 *       string, even if it represents a cryptographically weak key.
 */
void test_io(void) {
  uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
  /* Generate a valid key pair to use throughout the tests. */
  age_generate_keypair(pk, sk);

  /* -----------------------------------------------------------------
   * Public key tests
   * ----------------------------------------------------------------- */

  /* 1. Successful encoding */
  char pub_buf[AGE_PUBLIC_KEY_BUF_SIZE];
  TEST_EQ(age_public_key_to_string(pk, pub_buf), AGE_OK);
  /* Length must be exactly AGE_PUBLIC_KEY_STRING_LENGTH (56). */
  TEST_EQ(strlen(pub_buf), AGE_PUBLIC_KEY_STRING_LENGTH);
  /* The first 4 characters must be "age1". */
  TEST(strncmp(pub_buf, "age1", 4) == 0);

  /* 2. Round‑trip: decode the freshly encoded string and compare */
  uint8_t pk2[AGE_KEY_BYTES];
  TEST_EQ(age_string_to_public_key(pub_buf, pk2), AGE_OK);
  TEST(memcmp(pk, pk2, AGE_KEY_BYTES) == 0);

  /* 3. Invalid inputs – string too short (only the prefix) */
  char short_pub[AGE_PUBLIC_KEY_BUF_SIZE] = "age1";
  TEST_EQ(age_string_to_public_key(short_pub, pk2), AGE_ERR_INVALID_FORMAT);

  /* 4. Wrong prefix – "Xge1" instead of "age1" */
  char bad_prefix[AGE_PUBLIC_KEY_BUF_SIZE] = "Xge1";
  /* Fill the remaining 52 characters with a valid character ('q') so that
     the overall length is correct, only the prefix is wrong. */
  memset(bad_prefix + 4, 'q', 52);
  bad_prefix[56] = '\0';
  TEST_EQ(age_string_to_public_key(bad_prefix, pk2), AGE_ERR_INVALID_FORMAT);

  /* 5. Invalid character in the Bech32 part – 'x' is not in the charset */
  char bad_char[AGE_PUBLIC_KEY_BUF_SIZE] = "age1";
  memset(bad_char + 4, 'x', 52);
  bad_char[56] = '\0';
  TEST_EQ(age_string_to_public_key(bad_char, pk2), AGE_ERR_INVALID_FORMAT);

  /* 6. Weak public key – the age specification forbids an all‑zero key.
     Encode a zero key, then attempt to decode; the library must return
     AGE_ERR_WEAK_PUBLIC_KEY. */
  uint8_t zero_pk[AGE_KEY_BYTES] = {0};
  char zero_str[AGE_PUBLIC_KEY_BUF_SIZE];
  age_public_key_to_string(zero_pk, zero_str);
  TEST_EQ(age_string_to_public_key(zero_str, pk2), AGE_ERR_WEAK_PUBLIC_KEY);

  /* -----------------------------------------------------------------
   * Secret key tests
   * ----------------------------------------------------------------- */

  /* 7. Successful encoding */
  char sec_buf[AGE_SECRET_KEY_BUF_SIZE];
  TEST_EQ(age_secret_key_to_string(sk, sec_buf), AGE_OK);
  /* Length must be exactly AGE_SECRET_KEY_STRING_LENGTH (67). */
  TEST_EQ(strlen(sec_buf), AGE_SECRET_KEY_STRING_LENGTH);
  /* The prefix must be "AGE-SECRET-KEY-1". */
  TEST(strncmp(sec_buf, "AGE-SECRET-KEY-1", 15) == 0);

  /* 8. Round‑trip for secret key */
  uint8_t sk2[AGE_KEY_BYTES];
  TEST_EQ(age_string_to_secret_key(sec_buf, sk2), AGE_OK);
  TEST(memcmp(sk, sk2, AGE_KEY_BYTES) == 0);

  /* 9. Invalid inputs – string too short (only the prefix) */
  char short_sec[AGE_SECRET_KEY_BUF_SIZE] = "AGE-SECRET-KEY-1";
  TEST_EQ(age_string_to_secret_key(short_sec, sk2), AGE_ERR_INVALID_FORMAT);

  /* 10. Wrong prefix – "AGE-SECRET-KEY-2" */
  char bad_sec[AGE_SECRET_KEY_BUF_SIZE] = "AGE-SECRET-KEY-2";
  memset(bad_sec + 15, 'q', 52);
  bad_sec[67] = '\0';
  TEST_EQ(age_string_to_secret_key(bad_sec, sk2), AGE_ERR_INVALID_FORMAT);

  /* 11. Invalid character in the Bech32 payload */
  char bad_sec_char[AGE_SECRET_KEY_BUF_SIZE] = "AGE-SECRET-KEY-1";
  memset(bad_sec_char + 15, 'x', 52);
  bad_sec_char[67] = '\0';
  TEST_EQ(age_string_to_secret_key(bad_sec_char, sk2), AGE_ERR_INVALID_FORMAT);
}
