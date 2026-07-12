/**
 * @file gen.c
 * @brief Key generation and public key derivation for libcagekeygen.
 *
 * This file implements the core cryptographic operations of the library:
 * secure random number generation and X25519 scalar multiplication.
 *
 * Two public API functions are provided:
 * - `age_generate_keypair()` – creates a fresh random key pair.
 * - `age_public_key_from_secret_key()` – derives the public key from an
 *   existing secret key.
 *
 * Both functions rely on `curve25519_donna()` (an external implementation
 * of the Curve25519 function) to perform the elliptic‑curve math.
 *
 * Secure randomness is obtained through platform‑specific OS interfaces:
 * - Windows:        `BCryptGenRandom()`
 * - Linux:          `getrandom()`
 * - BSD/macOS:      `/dev/urandom` via `read()`
 *
 * All internal helpers are marked `static` and are not visible outside
 * this translation unit.
 *
 * @note The secret keys generated here are **not** pre‑clamped; the
 *       necessary clamping is performed inside `curve25519_donna()` during
 *       scalar multiplication.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 *
 * @see libcagekeygen.h for the public API declarations.
 * @see curve25519-donna.c for the elliptic‑curve implementation.
 */

#include "libcagekeygen.h"
#include <errno.h>
#include <string.h>

/* External function from curve25519-donna.c */
extern void curve25519_donna(uint8_t *out, const uint8_t *scalar,
                             const uint8_t *base);

/**
 * @brief Check whether a byte buffer consists entirely of zeros.
 *
 * Used to detect weak (all‑zero) public keys, which are forbidden by the
 * age specification and cryptographically worthless.
 *
 * @param[in] buf  Pointer to the buffer to examine.
 * @param[in] len  Number of bytes in the buffer.
 *
 * @return 1 if every byte in the buffer is zero, 0 otherwise.
 */
static int is_all_zero(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (buf[i] != 0)
      return 0;
  }
  return 1;
}

/* -----------------------------------------------------------------------
 * Platform‑specific secure random number generation.
 *
 * Each variant fills `buf` with `len` cryptographically strong random bytes
 * and returns 0 on success, -1 on failure. The function handles partial
 * reads / EINTR internally so that callers receive a simple success/failure
 * indication.
 * ----------------------------------------------------------------------- */

#if defined(_WIN32)
#include <bcrypt.h>
#include <windows.h>

/**
 * @brief Obtain secure random bytes using the Windows CNG API.
 *
 * Calls `BCryptGenRandom()` with the `BCRYPT_USE_SYSTEM_PREFERRED_RNG`
 * flag to request the system's preferred random number generator.
 *
 * @param[out] buf  Buffer to fill with random bytes. Must not be NULL.
 * @param[in]  len  Number of bytes to generate. Must be > 0.
 *
 * @return 0 on success, -1 on error (e.g., NULL pointer or CNG failure).
 */
static int secure_random(uint8_t *buf, size_t len) {
  if (!buf || len == 0)
    return -1;
  return BCryptGenRandom(NULL, buf, (ULONG)len,
                         BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0
             ? 0
             : -1;
}

#elif defined(__linux__)
#include <sys/random.h>

/**
 * @brief Obtain secure random bytes using the Linux `getrandom()` syscall.
 *
 * The function loops until `len` bytes have been read, retrying on
 * `EINTR` (which can occur if a signal is delivered during the call).
 * It does **not** fall back to `/dev/urandom` because `getrandom()`
 * is available on all modern Linux kernels (≥ 3.17) that are likely to
 * run age‑compatible software.
 *
 * @param[out] buf  Buffer to fill with random bytes. Must not be NULL.
 * @param[in]  len  Number of bytes to generate. Must be > 0.
 *
 * @return 0 on success, -1 if the syscall fails with a non‑EINTR error
 *         or if the input parameters are invalid.
 */
static int secure_random(uint8_t *buf, size_t len) {
  if (!buf || len == 0)
    return -1;
  ssize_t n;
  size_t total = 0;
  while (total < len) {
    n = getrandom(buf + total, len - total, 0);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      return -1;
    }
    total += n;
  }
  return 0;
}

#else /* BSD/macOS fallback */
#include <fcntl.h>
#include <unistd.h>

/**
 * @brief Obtain secure random bytes by reading from `/dev/urandom`.
 *
 * This fallback covers BSDs, macOS, and other Unix‑like systems that lack
 * the `getrandom()` syscall. The file descriptor is opened and closed on
 * every call to keep the function self‑contained and reentrant.
 *
 * @param[out] buf  Buffer to fill with random bytes. Must not be NULL.
 * @param[in]  len  Number of bytes to generate. Must be > 0.
 *
 * @return 0 on success, -1 if `/dev/urandom` cannot be opened or if a
 *         read error (other than `EINTR`) occurs.
 */
static int secure_random(uint8_t *buf, size_t len) {
  if (!buf || len == 0)
    return -1;
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0)
    return -1;
  ssize_t n;
  size_t total = 0;
  while (total < len) {
    n = read(fd, buf + total, len - total);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      close(fd);
      return -1;
    }
    total += n;
  }
  close(fd);
  return 0;
}
#endif

/* -----------------------------------------------------------------------
 * Public API functions
 * ----------------------------------------------------------------------- */

/**
 * @brief Generate a new X25519 key pair.
 *
 * Fills the `secret_key` buffer with 32 bytes of cryptographically secure
 * random data (obtained from the platform‑specific `secure_random()`),
 * then computes the corresponding public key as
 * `curve25519(secret_key, basepoint)`. The basepoint is the standard
 * Curve25519 generator (9).
 *
 * If the resulting public key is all zeros (an extremely unlikely event
 * that would indicate a degenerate secret key), the function returns
 * `AGE_ERR_KEYGEN_FAILED`. The caller can retry; a fresh call will use
 * a new random secret.
 *
 * @param[out] public_key   Buffer of exactly #AGE_KEY_BYTES where the
 *                          new public key will be stored. Must not be NULL.
 * @param[out] secret_key   Buffer of exactly #AGE_KEY_BYTES where the
 *                          new secret key will be stored. Must not be NULL.
 *
 * @return #AGE_OK on success.
 * @return #AGE_ERR_RANDOM_FAILED if `secure_random()` failed (e.g., lack of
 *         entropy or OS error).
 * @return #AGE_ERR_KEYGEN_FAILED if the computed public key is all zeros.
 *
 * @warning The function itself does **not** clamp the secret key; the
 *          necessary bit‑twiddling is done inside `curve25519_donna()`.
 *          Therefore, the raw secret key can be used directly with other
 *          X25519 implementations that also perform their own clamping.
 *
 * Example:
 * @code
 *   uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
 *   age_error_t err = age_generate_keypair(pk, sk);
 *   if (err != AGE_OK) {
 *       fprintf(stderr, "Key generation failed: %s\n", age_error_string(err));
 *   }
 * @endcode
 */
age_error_t age_generate_keypair(uint8_t public_key[static AGE_KEY_BYTES],
                                 uint8_t secret_key[static AGE_KEY_BYTES]) {
  /* 1. Obtain 32 random bytes for the secret key. */
  if (secure_random(secret_key, AGE_KEY_BYTES) != 0)
    return AGE_ERR_RANDOM_FAILED;

  /* 2. Compute public key = [secret_key] * G  (G = 9) */
  static const uint8_t basepoint[AGE_KEY_BYTES] = {9};
  curve25519_donna(public_key, secret_key, basepoint);

  /* 3. Reject the degenerate case of an all‑zero public key. */
  if (is_all_zero(public_key, AGE_KEY_BYTES))
    return AGE_ERR_KEYGEN_FAILED;

  return AGE_OK;
}

/**
 * @brief Derive the public key that corresponds to an existing secret key.
 *
 * Performs the same scalar multiplication as `age_generate_keypair()`,
 * but uses a caller‑supplied secret key instead of generating a fresh one.
 * The secret key is assumed to be a 32‑byte value (typically previously
 * generated by `age_generate_keypair()` or decoded from an age string).
 *
 * The function checks that the resulting public key is not all zeros; if
 * it is, the secret key is considered invalid/weak and
 * `AGE_ERR_KEYGEN_FAILED` is returned.
 *
 * @param[in]  secret_key   The 32‑byte raw secret key. Must not be NULL.
 * @param[out] public_key   Buffer of #AGE_KEY_BYTES that will receive the
 *                          derived public key. Must not be NULL.
 *
 * @return #AGE_OK on success.
 * @return #AGE_ERR_KEYGEN_FAILED if the derived public key is all zeros,
 *         indicating a degenerate (e.g., all‑zero) secret key.
 *
 * @note This function is useful when you have a previously stored secret
 *       key (e.g., read from an age identity file) and need to recover or
 *       display the corresponding public key without re‑generating the pair.
 *
 * Example:
 * @code
 *   uint8_t sk[AGE_KEY_BYTES];
 *   // ... load sk from somewhere ...
 *   uint8_t pk[AGE_KEY_BYTES];
 *   age_error_t err = age_public_key_from_secret_key(sk, pk);
 *   if (err != AGE_OK) {
 *       fprintf(stderr, "Invalid secret key: %s\n", age_error_string(err));
 *   }
 * @endcode
 */
age_error_t
age_public_key_from_secret_key(const uint8_t secret_key[static AGE_KEY_BYTES],
                               uint8_t public_key[static AGE_KEY_BYTES]) {
  /* 1. Compute public key = [secret_key] * G */
  static const uint8_t basepoint[AGE_KEY_BYTES] = {9};
  curve25519_donna(public_key, secret_key, basepoint);

  /* 2. Reject all‑zero public key (weak key) */
  if (is_all_zero(public_key, AGE_KEY_BYTES))
    return AGE_ERR_KEYGEN_FAILED;

  return AGE_OK;
}
