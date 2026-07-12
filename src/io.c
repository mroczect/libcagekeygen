/**
 * @file io.c
 * @brief String encoding and decoding for age‑compatible key representations.
 *
 * This file converts between raw 32‑byte X25519 keys and the standard age
 * string format. The encoding uses a custom Bech32 variant (without
 * checksum) over the alphabet `qpzry9x8gf2tvdw0s3jn54khce6mua7l`.
 *
 * Public API functions provided:
 * - `age_public_key_to_string()`  – raw public key → `"age1..."` string
 * - `age_secret_key_to_string()`  – raw secret key → `"AGE-SECRET-KEY-1..."`
 * - `age_string_to_public_key()`  – `"age1..."` string → raw public key
 * - `age_string_to_secret_key()`  – `"AGE-SECRET-KEY-1..."` string → raw secret
 * key
 *
 * All validation (prefix, length, character set, weak‑key checks) is
 * performed inside the decoding functions so that callers receive a
 * meaningful error code rather than silently corrupted data.
 *
 * The internal Bech32 encoder/decoder operates on fixed‑size input/output
 * arrays (32 bytes ↔ 52 characters) and does not allocate memory.
 *
 * @note The library intentionally omits the Bech32 checksum. This matches
 *       the age specification, which uses a bare encoding without a
 *       checksum because the key material itself is self‑authenticating
 *       (especially after an X25519 exchange).
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 *
 * @see libcagekeygen.h for the public declarations.
 */

#include "libcagekeygen.h"
#include <string.h>

/**
 * @brief The Bech32 character set used by age (no checksum).
 *
 * This 32‑character string defines the mapping from 5‑bit values to
 * printable ASCII characters. The order is identical to the one specified
 * in the age documentation.
 */
static const char CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

/**
 * @brief Encode a 32‑byte array into a 52‑character Bech32 string.
 *
 * Converts the 256‑bit input (interpreted as a big‑endian bit‑stream) into
 * 52 characters by repeatedly extracting 5‑bit groups. Any leftover bits
 * (when `AGE_KEY_BYTES * 8` is not a multiple of 5) are padded to a full
 * 5‑bit group with zeros.
 *
 * @param[in]  data  Exactly 32 bytes to encode.
 * @param[out] out   Buffer of exactly 52 characters (no null terminator
 *                   appended). The caller is responsible for ensuring it
 *                   is large enough.
 *
 * @note No input validation is performed; the caller must guarantee that
 *       `data` and `out` are non‑NULL and of the correct size.
 */
static void bech32_encode(const uint8_t data[static AGE_KEY_BYTES],
                          char out[static 52]) {
  unsigned int acc = 0;
  int bits = 0, idx = 0;
  for (int i = 0; i < AGE_KEY_BYTES; i++) {
    acc = (acc << 8) | data[i];
    bits += 8;
    while (bits >= 5) {
      bits -= 5;
      out[idx++] = CHARSET[(acc >> bits) & 0x1F];
    }
  }
  /* Flush remaining bits (if any) with zero‑padding. */
  if (bits > 0)
    out[idx++] = CHARSET[(acc << (5 - bits)) & 0x1F];
}

/**
 * @brief Decode a 52‑character Bech32 string back to a 32‑byte array.
 *
 * Converts each 5‑bit character back to bits and reconstructs the original
 * 256‑bit value. Returns -1 if any character is not in the allowed charset,
 * or if the resulting output does not fill exactly 32 bytes.
 *
 * @param[in]  in   Exactly 52 characters from the Bech32 alphabet.
 * @param[out] out  Buffer of at least 32 bytes to receive the decoded key.
 *
 * @return 0 on success, -1 on error (invalid character or incorrect output
 *         length).
 */
static int bech32_decode(const char in[static 52],
                         uint8_t out[static AGE_KEY_BYTES]) {
  unsigned int acc = 0;
  int bits = 0, out_idx = 0;
  for (int i = 0; i < 52; i++) {
    const char *p = strchr(CHARSET, in[i]);
    if (!p)
      return -1; /* character not in the allowed set */
    acc = (acc << 5) | (unsigned int)(p - CHARSET);
    bits += 5;
    if (bits >= 8) {
      bits -= 8;
      out[out_idx++] = (acc >> bits) & 0xFF;
    }
  }
  /* Must have collected exactly 32 bytes. */
  return (out_idx == AGE_KEY_BYTES) ? 0 : -1;
}

/**
 * @brief Check whether a byte buffer consists entirely of zeros.
 *
 * Used to detect the forbidden all‑zero public key. Returns 1 if every
 * byte in the buffer is zero, 0 otherwise.
 *
 * @param[in] buf  Pointer to the buffer to examine.
 * @param[in] len  Number of bytes in the buffer.
 *
 * @return 1 if all bytes are zero, 0 otherwise.
 */
static int is_all_zero(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++)
    if (buf[i] != 0)
      return 0;
  return 1;
}

/**
 * @brief Encode a raw public key into the age string format.
 *
 * Writes the null‑terminated string `"age1"` followed by 52 Bech32
 * characters into the output buffer. The buffer must be at least
 * #AGE_PUBLIC_KEY_BUF_SIZE bytes long.
 *
 * @param[in]  public_key  32‑byte raw public key. Must not be NULL.
 * @param[out] buf         Output buffer of at least #AGE_PUBLIC_KEY_BUF_SIZE
 *                         bytes. On return, contains the null‑terminated
 *                         public key string (length =
 * #AGE_PUBLIC_KEY_STRING_LENGTH).
 *
 * @return Always returns #AGE_OK (no error conditions in the current
 *         implementation).
 *
 * @see age_string_to_public_key() for the reverse operation.
 */
age_error_t
age_public_key_to_string(const uint8_t public_key[static AGE_KEY_BYTES],
                         char buf[static AGE_PUBLIC_KEY_BUF_SIZE]) {
  /* Prefix "age1" (4 bytes) */
  memcpy(buf, "age1", 4);
  /* Encode the 32‑byte key into the following 52 characters */
  bech32_encode(public_key, buf + 4);
  /* Terminate the string */
  buf[AGE_PUBLIC_KEY_STRING_LENGTH] = '\0';
  return AGE_OK;
}

/**
 * @brief Encode a raw secret key into the age secret key string format.
 *
 * Writes the null‑terminated string `"AGE-SECRET-KEY-1"` followed by 52
 * Bech32 characters into the output buffer. The buffer must be at least
 * #AGE_SECRET_KEY_BUF_SIZE bytes.
 *
 * @param[in]  secret_key  32‑byte raw secret key. Must not be NULL.
 * @param[out] buf         Output buffer of at least #AGE_SECRET_KEY_BUF_SIZE
 *                         bytes. On return, contains the null‑terminated
 *                         secret key string (length =
 * #AGE_SECRET_KEY_STRING_LENGTH).
 *
 * @return Always returns #AGE_OK.
 *
 * @see age_string_to_secret_key() for the reverse operation.
 */
age_error_t
age_secret_key_to_string(const uint8_t secret_key[static AGE_KEY_BYTES],
                         char buf[static AGE_SECRET_KEY_BUF_SIZE]) {
  const char *prefix = "AGE-SECRET-KEY-";
  size_t plen = strlen(prefix); /* 14 */
  memcpy(buf, prefix, plen);
  buf[plen] = '1'; /* "AGE-SECRET-KEY-1" */
  /* Encode into the 52 bytes after the prefix */
  bech32_encode(secret_key, buf + plen + 1);
  buf[AGE_SECRET_KEY_STRING_LENGTH] = '\0';
  return AGE_OK;
}

/**
 * @brief Decode an age public key string back into a raw 32‑byte key.
 *
 * The input string must be exactly #AGE_PUBLIC_KEY_STRING_LENGTH characters
 * long and match the format `"age1"` + 52 Bech32 characters.
 *
 * The function performs the following checks in order:
 * 1. Exact length (56 characters).
 * 2. Correct prefix (`"age1"`).
 * 3. Valid Bech32 characters in the remaining 52 positions.
 * 4. The decoded key is not all‑zero (which would be a weak, forbidden key).
 *
 * @param[in]  str         Null‑terminated input string. Must have exactly
 *                         #AGE_PUBLIC_KEY_STRING_LENGTH characters (plus
 *                         the terminator).
 * @param[out] public_key  Buffer of #AGE_KEY_BYTES where the decoded raw
 *                         key will be stored. Must not be NULL.
 *
 * @return #AGE_OK on success.
 * @return #AGE_ERR_INVALID_FORMAT if the string fails any of the format
 *         checks.
 * @return #AGE_ERR_WEAK_PUBLIC_KEY if the decoded public key is all zeros.
 */
age_error_t age_string_to_public_key(
    const char str[static AGE_PUBLIC_KEY_STRING_LENGTH + 1],
    uint8_t public_key[static AGE_KEY_BYTES]) {
  /* Check total length */
  if (strlen(str) != AGE_PUBLIC_KEY_STRING_LENGTH)
    return AGE_ERR_INVALID_FORMAT;
  /* Verify prefix */
  if (strncmp(str, "age1", 4) != 0)
    return AGE_ERR_INVALID_FORMAT;
  /* Decode Bech32 part */
  if (bech32_decode(str + 4, public_key) != 0)
    return AGE_ERR_INVALID_FORMAT;
  /* Reject weak (all‑zero) key */
  if (is_all_zero(public_key, AGE_KEY_BYTES))
    return AGE_ERR_WEAK_PUBLIC_KEY;
  return AGE_OK;
}

/**
 * @brief Decode an age secret key string back into a raw 32‑byte key.
 *
 * The input string must be exactly #AGE_SECRET_KEY_STRING_LENGTH characters
 * long and start with `"AGE-SECRET-KEY-1"` followed by 52 Bech32 characters.
 *
 * The function validates:
 * 1. Exact length (67 characters).
 * 2. Correct prefix (`"AGE-SECRET-KEY-1"`).
 * 3. The remaining 52 characters are valid Bech32 symbols.
 *
 * No check for weak keys is performed on secret keys – an all‑zero secret
 * key, although cryptographically invalid, will be decoded faithfully.
 *
 * @param[in]  str         Null‑terminated input string. Must have exactly
 *                         #AGE_SECRET_KEY_STRING_LENGTH characters (plus
 *                         terminator).
 * @param[out] secret_key  Buffer of #AGE_KEY_BYTES to receive the decoded
 *                         raw key. Must not be NULL.
 *
 * @return #AGE_OK on success.
 * @return #AGE_ERR_INVALID_FORMAT if any format check fails.
 */
age_error_t age_string_to_secret_key(
    const char str[static AGE_SECRET_KEY_STRING_LENGTH + 1],
    uint8_t secret_key[static AGE_KEY_BYTES]) {
  /* Check total length */
  if (strlen(str) != AGE_SECRET_KEY_STRING_LENGTH)
    return AGE_ERR_INVALID_FORMAT;
  /* Verify prefix ("AGE-SECRET-KEY-1" = 15 chars) */
  const char *prefix = "AGE-SECRET-KEY-1";
  if (strncmp(str, prefix, 15) != 0)
    return AGE_ERR_INVALID_FORMAT;
  /* Decode the 52 Bech32 characters */
  if (bech32_decode(str + 15, secret_key) != 0)
    return AGE_ERR_INVALID_FORMAT;
  return AGE_OK;
}
