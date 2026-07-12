/**
 * @file error.c
 * @brief Implementation of `age_error_string()` – human-readable error
 *        descriptions for the libcagekeygen library.
 *
 * This file contains the single function responsible for converting the
 * library's error codes (enum `age_error_t`) into constant, null-terminated
 * English strings suitable for logging or displaying to the user.
 *
 * The function uses a `switch` statement without a `default` fall-through
 * to a generic message, ensuring that even unexpected (e.g., corrupted)
 * error values are handled gracefully.
 *
 * @note The returned strings are static and must NOT be freed or modified
 *       by the caller.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 *
 * @see libcagekeygen.h for the definition of `age_error_t`.
 */

#include "libcagekeygen.h"

/**
 * @brief Returns a static, human-readable string describing an error code.
 *
 * This function maps every possible value of `age_error_t` (including
 * success) to a short English description. The returned pointer refers
 * to a string literal stored in read-only memory; it must not be
 * deallocated or modified.
 *
 * @param err   Error code returned by any library function. Passing a
 *              value that is not part of the `age_error_t` enumeration
 *              results in the generic string `"unknown error"`.
 *
 * @return Pointer to a null-terminated C string. The string is valid
 *         for the lifetime of the program.
 *
 * @warning Do not attempt to free the returned pointer or write to the
 *          memory it points to.
 *
 * Example usage:
 * @code
 *   age_error_t ret = age_generate_keypair(pk, sk);
 *   if (ret != AGE_OK) {
 *       fprintf(stderr, "Key generation failed: %s\n", age_error_string(ret));
 *   }
 * @endcode
 */
const char *age_error_string(age_error_t err) {
  /**
   * Each case directly corresponds to an enum value in `age_error_t`.
   * The labels are intentionally listed exhaustively so that a compiler
   * warning (`-Wswitch`) can catch newly added enum values that might
   * be forgotten here.
   */
  switch (err) {
  case AGE_OK:
    return "no error"; /**< Operation completed successfully. */
  case AGE_ERR_NULL_POINTER:
    return "null pointer argument"; /**< A required pointer was NULL (reserved).
                                     */
  case AGE_ERR_RANDOM_FAILED:
    return "failed to generate random bytes"; /**< The system's random source
                                                 failed. */
  case AGE_ERR_BUFFER_TOO_SMALL:
    return "output buffer too small"; /**< Provided buffer is insufficient
                                         (reserved). */
  case AGE_ERR_INVALID_FORMAT:
    return "invalid age string format"; /**< Input string does not match the
                                           expected format. */
  case AGE_ERR_KEYGEN_FAILED:
    return "key generation failed (weak key)"; /**< Key generation produced an
                                                  all‑zero public key. */
  case AGE_ERR_WEAK_PUBLIC_KEY:
    return "public key is weak (all zeros)"; /**< Decoded public key is the
                                                forbidden all‑zero value. */
  /**
   * The default case handles any unknown code – either because the caller
   * passed an integer that does not belong to the enum, or because a new
   * error code was added to the library but this function wasn't updated.
   * Returning a generic string prevents undefined behaviour (e.g., missing
   * return statement) and gives a clue to the developer.
   */
  default:
    return "unknown error";
  }
}
