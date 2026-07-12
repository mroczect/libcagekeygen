/**
 * @file libcagekeygen.h
 * @brief Public API for libcagekeygen – an age-compatible X25519 key generation
 *        and string encoding/decoding library.
 *
 * This header defines all types, constants, error codes, and functions needed
 * to generate Curve25519 key pairs, derive public keys from existing secrets,
 * and convert between raw 32-byte keys and the standard age string
 * representations (Bech32 encoding with `age1` or `AGE-SECRET-KEY-1` prefixes).
 *
 * The library is written in standard C99, does **not** perform dynamic memory
 * allocation, and all functions operate on caller-provided buffers whose
 * required sizes are fully described by the macros below.
 *
 * @note The actual implementation of the Curve25519 scalar multiplication is
 *       provided by the included `curve25519-donna` module.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3 
 *
 * Usage example:
 * @code
 *   #include "libcagekeygen.h"
 *   uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
 *   age_error_t err = age_generate_keypair(pk, sk);
 *   if (err != AGE_OK) { ... }
 *   char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
 *   age_public_key_to_string(pk, pub_str);
 *   printf("Public: %s\n", pub_str);
 * @endcode
 */

#ifndef LIBCAGEKEYGEN_H
#define LIBCAGEKEYGEN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Size of a raw Curve25519 key in bytes (256 bits).
 *
 * Both secret keys and public keys are exactly 32 bytes long. This value is
 * the fundamental building block for all buffer sizes in this API.
 */
#define AGE_KEY_BYTES 32

/**
 * @brief Length of the printable public key string (excluding null terminator).
 *
 * The format is `"age1"` followed by 52 characters from the Bech32 alphabet,
 * giving a total of 56 characters.
 *
 * @see AGE_PUBLIC_KEY_BUF_SIZE for the required buffer size including the null
 *      terminator.
 */
#define AGE_PUBLIC_KEY_STRING_LENGTH  56   // "age1" + 52

/**
 * @brief Length of the printable secret key string (excluding null terminator).
 *
 * The format is `"AGE-SECRET-KEY-1"` followed by 52 Bech32 characters,
 * giving a total of 67 characters.
 *
 * @see AGE_SECRET_KEY_BUF_SIZE for the required buffer size including the null
 *      terminator.
 */
#define AGE_SECRET_KEY_STRING_LENGTH  67   // "AGE-SECRET-KEY-1" + 52

/**
 * @brief Minimum buffer size (in bytes) for a null-terminated public key string.
 *
 * Always equal to `AGE_PUBLIC_KEY_STRING_LENGTH + 1` (i.e. 57). Use this
 * constant when declaring char arrays that will hold a public key string.
 *
 * @code
 *   char buf[AGE_PUBLIC_KEY_BUF_SIZE];
 *   age_public_key_to_string(pk, buf);
 * @endcode
 */
#define AGE_PUBLIC_KEY_BUF_SIZE  (AGE_PUBLIC_KEY_STRING_LENGTH + 1)  // 57

/**
 * @brief Minimum buffer size (in bytes) for a null-terminated secret key string.
 *
 * Always equal to `AGE_SECRET_KEY_STRING_LENGTH + 1` (i.e. 68). Use this
 * constant when declaring char arrays that will hold a secret key string.
 *
 * @code
 *   char buf[AGE_SECRET_KEY_BUF_SIZE];
 *   age_secret_key_to_string(sk, buf);
 * @endcode
 */
#define AGE_SECRET_KEY_BUF_SIZE  (AGE_SECRET_KEY_STRING_LENGTH + 1)  // 68

/**
 * @brief Error codes returned by all library functions.
 *
 * All functions return a value of this type. `AGE_OK` indicates success;
 * negative values indicate specific failures.
 */
typedef enum {
    AGE_OK                  =  0,  /**< Operation completed successfully. */
    AGE_ERR_NULL_POINTER    = -1,  /**< A required pointer argument was NULL (reserved). */
    AGE_ERR_RANDOM_FAILED   = -2,  /**< Unable to obtain secure random bytes from the OS. */
    AGE_ERR_BUFFER_TOO_SMALL = -3, /**< Output buffer is too small to hold the result (reserved). */
    AGE_ERR_INVALID_FORMAT  = -4,  /**< Input string does not match the expected age format. */
    AGE_ERR_KEYGEN_FAILED   = -5,  /**< Key generation produced a weak (all-zero) public key. */
    AGE_ERR_WEAK_PUBLIC_KEY = -6,  /**< Decoded public key is the forbidden all-zero value. */
} age_error_t;

/**
 * @brief Generate a new X25519 key pair.
 *
 * Fills the @p secret_key buffer with 32 bytes of cryptographically secure
 * randomness and computes the corresponding public key by performing
 * `X25519(secret_key, basepoint)`. The resulting public key is stored in
 * @p public_key.
 *
 * @param[out] public_key   Buffer of exactly #AGE_KEY_BYTES to receive the
 *                          new public key. Must not be NULL.
 * @param[out] secret_key   Buffer of exactly #AGE_KEY_BYTES to receive the
 *                          new secret key. Must not be NULL.
 *
 * @return #AGE_OK on success.
 * @return #AGE_ERR_RANDOM_FAILED if the system random source (e.g.,
 *         `/dev/urandom`, `getrandom()`, or `BCryptGenRandom`) fails.
 * @return #AGE_ERR_KEYGEN_FAILED if the scalar multiplication yields an
 *         all-zero public key (extremely unlikely but checked for safety).
 *
 * @note The secret key generated here is **not** clamped (the required
 *       clamping for X25519 is performed internally by the Donna
 *       implementation during scalar multiplication). It can be used directly
 *       with `age_public_key_from_secret_key()` or stored in age string format.
 */
age_error_t age_generate_keypair(
    uint8_t public_key[static AGE_KEY_BYTES],
    uint8_t secret_key[static AGE_KEY_BYTES]);

/**
 * @brief Derive the public key that corresponds to an existing secret key.
 *
 * Computes the X25519 scalar multiplication `public_key = [secret_key]G`,
 * where `G` is the standard curve25519 base point.
 *
 * @param[in]  secret_key   The 32-byte secret key. Must not be NULL.
 * @param[out] public_key   Buffer of exactly #AGE_KEY_BYTES where the
 *                          derived public key will be written. Must not be NULL.
 *
 * @return #AGE_OK if the derivation succeeded.
 * @return #AGE_ERR_KEYGEN_FAILED if the resulting public key is all zeros,
 *         which indicates a degenerate (weak) secret key (e.g., all zeros).
 *
 * @warning The function itself does **not** check the secret key for validity
 *          beyond the final public key. An all-zero secret key is an invalid
 *          input according to the Curve25519 specification, and this function
 *          will detect it by the zero output and return an error.
 */
age_error_t age_public_key_from_secret_key(
    const uint8_t secret_key[static AGE_KEY_BYTES],
    uint8_t public_key[static AGE_KEY_BYTES]);

/**
 * @brief Encode a raw 32-byte public key into the age string format.
 *
 * Produces a null-terminated string of the form `age1` + 52 Bech32 characters.
 * The output buffer must be at least #AGE_PUBLIC_KEY_BUF_SIZE bytes long.
 *
 * @param[in]  public_key   The 32-byte raw public key.
 * @param[out] buf          Output buffer of at least #AGE_PUBLIC_KEY_BUF_SIZE
 *                          bytes. On return the buffer contains the
 *                          null-terminated public key string.
 *
 * @return Always returns #AGE_OK (no error conditions in the current
 *         implementation).
 *
 * @see age_string_to_public_key() for the reverse operation.
 */
age_error_t age_public_key_to_string(
    const uint8_t public_key[static AGE_KEY_BYTES],
    char buf[static AGE_PUBLIC_KEY_BUF_SIZE]);

/**
 * @brief Encode a raw 32-byte secret key into the age secret key string format.
 *
 * Produces a null-terminated string starting with `AGE-SECRET-KEY-1` followed
 * by 52 Bech32 characters. The output buffer must be at least
 * #AGE_SECRET_KEY_BUF_SIZE bytes.
 *
 * @param[in]  secret_key   The 32-byte raw secret key.
 * @param[out] buf          Output buffer of at least #AGE_SECRET_KEY_BUF_SIZE
 *                          bytes. On return the buffer contains the
 *                          null-terminated secret key string.
 *
 * @return Always returns #AGE_OK (no error conditions in the current
 *         implementation).
 *
 * @see age_string_to_secret_key() for the reverse operation.
 */
age_error_t age_secret_key_to_string(
    const uint8_t secret_key[static AGE_KEY_BYTES],
    char buf[static AGE_SECRET_KEY_BUF_SIZE]);

/**
 * @brief Decode an age public key string into a raw 32-byte key.
 *
 * The input string must be exactly #AGE_PUBLIC_KEY_STRING_LENGTH characters
 * long (excluding the null terminator) and follow the exact format:
 * - prefix `age1`
 * - 52 characters from the Bech32 alphabet (charset: `qpzry9x8gf2tvdw0s3jn54khce6mua7l`)
 *
 * The function performs the following checks:
 * 1. String length is exactly 56.
 * 2. The first four characters are `age1`.
 * 3. The remaining 52 characters are valid Bech32 symbols.
 * 4. The decoded raw key is **not** all zeros (forbidden by the age spec).
 *
 * @param[in]  str          Null-terminated input string. Must have exactly
 *                          #AGE_PUBLIC_KEY_STRING_LENGTH printable characters.
 * @param[out] public_key   Buffer of #AGE_KEY_BYTES bytes to hold the decoded key.
 *
 * @return #AGE_OK on successful decoding.
 * @return #AGE_ERR_INVALID_FORMAT if the string length, prefix, or character
 *         set is invalid.
 * @return #AGE_ERR_WEAK_PUBLIC_KEY if the decoded key is all zeros.
 */
age_error_t age_string_to_public_key(
    const char str[static AGE_PUBLIC_KEY_STRING_LENGTH + 1],
    uint8_t public_key[static AGE_KEY_BYTES]);

/**
 * @brief Decode an age secret key string into a raw 32-byte key.
 *
 * The input string must be exactly #AGE_SECRET_KEY_STRING_LENGTH characters
 * long and follow the format:
 * - prefix `AGE-SECRET-KEY-1`
 * - 52 Bech32 characters
 *
 * The function checks:
 * 1. String length is exactly 67.
 * 2. The first 15 characters are `AGE-SECRET-KEY-1`.
 * 3. The remaining 52 characters belong to the Bech32 alphabet.
 *
 * No check for weak (all-zero) keys is performed on secret keys – such a key
 * is considered a valid encoding, even though it would be cryptographically
 * unusable.
 *
 * @param[in]  str          Null-terminated input string. Must have exactly
 *                          #AGE_SECRET_KEY_STRING_LENGTH printable characters.
 * @param[out] secret_key   Buffer of #AGE_KEY_BYTES bytes to receive the decoded
 *                          secret key.
 *
 * @return #AGE_OK on success.
 * @return #AGE_ERR_INVALID_FORMAT if the string does not meet the format
 *         requirements.
 */
age_error_t age_string_to_secret_key(
    const char str[static AGE_SECRET_KEY_STRING_LENGTH + 1],
    uint8_t secret_key[static AGE_KEY_BYTES]);

/**
 * @brief Return a human-readable description of an error code.
 *
 * The returned string is a pointer to a static constant buffer; it must not be
 * freed or modified by the caller.
 *
 * @param err   An error code returned by any library function.
 *
 * @return Pointer to a null-terminated C string describing the error (e.g.,
 *         `"no error"`, `"failed to generate random bytes"`). For unknown
 *         codes, returns `"unknown error"`.
 *
 * @note The returned string is always in English.
 */
const char *age_error_string(age_error_t err);

#ifdef __cplusplus
}
#endif

#endif /* LIBCAGEKEYGEN_H */
