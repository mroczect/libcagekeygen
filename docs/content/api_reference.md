---
title: "API Reference"
desc: "Complete reference for all constants, types, and functions in libcagekeygen."
---

# API Reference

This page describes every public constant, type, and function provided by
libcagekeygen. All declarations are available in the single header
`libcagekeygen.h`.

---

## Constants

Buffer sizes are defined as preprocessor macros. Always use these macros
instead of hard‑coding numbers to ensure correct buffer sizes and forward
compatibility.

| Macro                          | Value | Description                                                                                                                     |
| ------------------------------ | ----- | ------------------------------------------------------------------------------------------------------------------------------- |
| `AGE_KEY_BYTES`                | `32`  | Size of a raw X25519 key in bytes (256 bits). Both public and secret keys are exactly 32 bytes.                                 |
| `AGE_PUBLIC_KEY_STRING_LENGTH` | `56`  | Length of a public key string in characters, **excluding** the null terminator. Format: `"age1"` + 52 Bech32 chars.             |
| `AGE_SECRET_KEY_STRING_LENGTH` | `67`  | Length of a secret key string in characters, **excluding** the null terminator. Format: `"AGE-SECRET-KEY-1"` + 52 Bech32 chars. |
| `AGE_PUBLIC_KEY_BUF_SIZE`      | `57`  | Minimum buffer size for a null‑terminated public key string (`LENGTH + 1`).                                                     |
| `AGE_SECRET_KEY_BUF_SIZE`      | `68`  | Minimum buffer size for a null‑terminated secret key string (`LENGTH + 1`).                                                     |

### Visual representation of the string formats

```
Public key string (56 characters)
┌──────┬────────────────────────────────────────────────────┐
│ age1 │ 52 characters from Bech32 alphabet                │
└──────┴────────────────────────────────────────────────────┘

Secret key string (67 characters)
┌──────────────────┬────────────────────────────────────────────────────┐
│ AGE-SECRET-KEY-1 │ 52 characters from Bech32 alphabet                │
└──────────────────┴────────────────────────────────────────────────────┘
```

Both strings are printable ASCII and do **not** contain any whitespace.

---

## Error Codes

All library functions return an `age_error_t` value. The possible codes are:

| Enum constant              | Value | Description                                                                                              |
| -------------------------- | ----- | -------------------------------------------------------------------------------------------------------- |
| `AGE_OK`                   | `0`   | Success.                                                                                                 |
| `AGE_ERR_NULL_POINTER`     | `-1`  | A required pointer argument was `NULL` (reserved; currently not returned).                               |
| `AGE_ERR_RANDOM_FAILED`    | `-2`  | The system's secure random source could not provide enough entropy.                                      |
| `AGE_ERR_BUFFER_TOO_SMALL` | `-3`  | An output buffer is too small to hold the result (reserved; not actively used).                          |
| `AGE_ERR_INVALID_FORMAT`   | `-4`  | Input string does not match the expected age format (wrong length, bad prefix, or illegal characters).   |
| `AGE_ERR_KEYGEN_FAILED`    | `-5`  | Key generation produced a weak (all‑zero) public key. This is extremely unlikely but checked for safety. |
| `AGE_ERR_WEAK_PUBLIC_KEY`  | `-6`  | A decoded public key is the forbidden all‑zero value.                                                    |

Use `age_error_string()` to obtain a human‑readable description of any code.

### Error handling best practices

Always check the return value of every function that can fail. Do not ignore
errors even for functions that "usually" succeed – especially key generation,
which depends on system entropy. A typical pattern:

```c
age_error_t err = age_generate_keypair(pk, sk);
if (err != AGE_OK) {
    fprintf(stderr, "Failed to generate key: %s\n", age_error_string(err));
    return EXIT_FAILURE;
}
```

---

## Functions

### `age_generate_keypair`

```c
age_error_t age_generate_keypair(
    uint8_t public_key[static AGE_KEY_BYTES],
    uint8_t secret_key[static AGE_KEY_BYTES]
);
```

**Description**  
Generates a fresh X25519 key pair. The secret key is filled with 32
cryptographically secure random bytes obtained from the operating system.
The corresponding public key is computed as `X25519(secret_key, basepoint)`,
where the basepoint is the standard Curve25519 generator (the value `9`).

If the resulting public key is all zeros (a degenerate case with
probability ≈ 2⁻²⁵⁶), the function returns `AGE_ERR_KEYGEN_FAILED` and
the output buffers should not be used. A fresh call will almost certainly
succeed.

**Parameters**

- `public_key` – Buffer of exactly `AGE_KEY_BYTES` bytes. On success,
  receives the newly generated public key.
- `secret_key` – Buffer of exactly `AGE_KEY_BYTES` bytes. On success,
  receives the newly generated secret key.

**Return values**

- `AGE_OK` – Success.
- `AGE_ERR_RANDOM_FAILED` – The system random source failed.
- `AGE_ERR_KEYGEN_FAILED` – The computed public key was all zeros (retry).

**Notes**

- The raw secret key is **not** clamped by this function. Clamping is
  performed internally by the Donna implementation during scalar
  multiplication. The raw bytes can be used directly with other X25519
  libraries that also clamp internally.
- The function does not allocate memory; all buffers are caller‑provided.
- The secret key is a 256‑bit integer in little‑endian byte order. You
  should never expose it or transmit it insecurely.

**Example**

```c
uint8_t pk[AGE_KEY_BYTES];
uint8_t sk[AGE_KEY_BYTES];
if (age_generate_keypair(pk, sk) != AGE_OK) {
    // handle error
}
```

---

### `age_public_key_from_secret_key`

```c
age_error_t age_public_key_from_secret_key(
    const uint8_t secret_key[static AGE_KEY_BYTES],
    uint8_t public_key[static AGE_KEY_BYTES]
);
```

**Description**  
Derives the public key that corresponds to an existing secret key. This
performs the same scalar multiplication as key generation, but uses a
caller‑supplied secret key instead of creating a new random one.

If the derived public key is all zeros, the function returns
`AGE_ERR_KEYGEN_FAILED` because the secret key is degenerate (e.g., all
zeros).

**Parameters**

- `secret_key` – A 32‑byte raw secret key. Must not be `NULL`.
- `public_key` – Buffer of exactly `AGE_KEY_BYTES` bytes. On success,
  receives the derived public key.

**Return values**

- `AGE_OK` – Success.
- `AGE_ERR_KEYGEN_FAILED` – The resulting public key is all zeros,
  indicating an invalid (weak) secret key.

**Example**

```c
uint8_t pk[AGE_KEY_BYTES];
if (age_public_key_from_secret_key(sk, pk) != AGE_OK) {
    // secret key is weak
}
```

---

### `age_public_key_to_string`

```c
age_error_t age_public_key_to_string(
    const uint8_t public_key[static AGE_KEY_BYTES],
    char buf[static AGE_PUBLIC_KEY_BUF_SIZE]
);
```

**Description**  
Encodes a raw 32‑byte public key into the standard age public key string
format. The resulting string is null‑terminated and has the form
`"age1"` followed by 52 Bech32 characters.

**Parameters**

- `public_key` – A 32‑byte raw public key.
- `buf` – Output buffer of at least `AGE_PUBLIC_KEY_BUF_SIZE` (57) bytes.
  On return, contains the null‑terminated string.

**Return values**

- Always returns `AGE_OK` (no error conditions in the current implementation).

**Example**

```c
char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
age_public_key_to_string(pk, pub_str);
printf("%s\n", pub_str);   // e.g., "age1..."
```

---

### `age_secret_key_to_string`

```c
age_error_t age_secret_key_to_string(
    const uint8_t secret_key[static AGE_KEY_BYTES],
    char buf[static AGE_SECRET_KEY_BUF_SIZE]
);
```

**Description**  
Encodes a raw 32‑byte secret key into the standard age secret key string
format. The string is null‑terminated and begins with
`"AGE-SECRET-KEY-1"` followed by 52 Bech32 characters.

**Parameters**

- `secret_key` – A 32‑byte raw secret key.
- `buf` – Output buffer of at least `AGE_SECRET_KEY_BUF_SIZE` (68) bytes.
  On return, contains the null‑terminated string.

**Return values**

- Always returns `AGE_OK`.

**Example**

```c
char sec_str[AGE_SECRET_KEY_BUF_SIZE];
age_secret_key_to_string(sk, sec_str);
printf("%s\n", sec_str);   // e.g., "AGE-SECRET-KEY-1..."
```

---

### `age_string_to_public_key`

```c
age_error_t age_string_to_public_key(
    const char str[static AGE_PUBLIC_KEY_STRING_LENGTH + 1],
    uint8_t public_key[static AGE_KEY_BYTES]
);
```

**Description**  
Decodes an age public key string back into a raw 32‑byte key. The function
performs thorough validation:

1. The string must be exactly `AGE_PUBLIC_KEY_STRING_LENGTH` (56) printable
   characters (not counting the null terminator).
2. The first four characters must be `age1`.
3. The remaining 52 characters must belong to the Bech32 alphabet
   (`qpzry9x8gf2tvdw0s3jn54khce6mua7l`).
4. The decoded raw key must **not** be all zeros (a forbidden weak key).

If any check fails, the function returns an appropriate error code and the
output buffer is left in an undefined state.

**Parameters**

- `str` – Null‑terminated input string with exactly 56 printable characters.
- `public_key` – Buffer of `AGE_KEY_BYTES` bytes to receive the decoded key.

**Return values**

- `AGE_OK` – Decoding successful.
- `AGE_ERR_INVALID_FORMAT` – The string does not match the expected format.
- `AGE_ERR_WEAK_PUBLIC_KEY` – The decoded key is all zeros.

**Example**

```c
uint8_t pk[AGE_KEY_BYTES];
age_error_t err = age_string_to_public_key("age1...", pk);
if (err != AGE_OK) {
    fprintf(stderr, "Invalid public key: %s\n", age_error_string(err));
}
```

---

### `age_string_to_secret_key`

```c
age_error_t age_string_to_secret_key(
    const char str[static AGE_SECRET_KEY_STRING_LENGTH + 1],
    uint8_t secret_key[static AGE_KEY_BYTES]
);
```

**Description**  
Decodes an age secret key string back into a raw 32‑byte key. Validation
checks:

1. The string must be exactly `AGE_SECRET_KEY_STRING_LENGTH` (67) printable
   characters.
2. The prefix must be exactly `AGE-SECRET-KEY-1` (15 characters).
3. The remaining 52 characters must be valid Bech32 symbols.

Unlike `age_string_to_public_key`, this function does **not** reject
all‑zero secret keys. An all‑zero secret key is considered a valid encoding,
though it would be cryptographically unusable.

**Parameters**

- `str` – Null‑terminated input string with exactly 67 printable characters.
- `secret_key` – Buffer of `AGE_KEY_BYTES` bytes to receive the decoded key.

**Return values**

- `AGE_OK` – Decoding successful.
- `AGE_ERR_INVALID_FORMAT` – The string fails any format check.

**Example**

```c
uint8_t sk[AGE_KEY_BYTES];
if (age_string_to_secret_key("AGE-SECRET-KEY-1...", sk) != AGE_OK) {
    // malformed input
}
```

---

### `age_error_string`

```c
const char *age_error_string(age_error_t err);
```

**Description**  
Returns a pointer to a static, null‑terminated string describing the given
error code. The string is in English and must not be freed or modified.

**Parameters**

- `err` – An error code returned by any library function.

**Return value**  
A pointer to a constant character string. Never returns `NULL`; unknown
error codes yield `"unknown error"`.

**Example**

```c
age_error_t ret = age_generate_keypair(pk, sk);
if (ret != AGE_OK) {
    printf("Error: %s\n", age_error_string(ret));
}
```

---

## Complete Example

Here’s a full program that demonstrates key generation, encoding, decoding,
and deriving a public key:

```c
#include <stdio.h>
#include <string.h>
#include "libcagekeygen.h"

int main(void) {
    // 1. Generate a fresh key pair
    uint8_t public_key[AGE_KEY_BYTES];
    uint8_t secret_key[AGE_KEY_BYTES];
    age_error_t err = age_generate_keypair(public_key, secret_key);
    if (err != AGE_OK) {
        fprintf(stderr, "Error generating key pair: %s\n", age_error_string(err));
        return 1;
    }

    // 2. Convert to strings
    char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
    char sec_str[AGE_SECRET_KEY_BUF_SIZE];
    age_public_key_to_string(public_key, pub_str);
    age_secret_key_to_string(secret_key, sec_str);

    printf("Public key:  %s\n", pub_str);
    printf("Secret key:  %s\n", sec_str);

    // 3. Decode the strings back to raw keys
    uint8_t decoded_pub[AGE_KEY_BYTES];
    uint8_t decoded_sec[AGE_KEY_BYTES];
    if (age_string_to_public_key(pub_str, decoded_pub) != AGE_OK ||
        age_string_to_secret_key(sec_str, decoded_sec) != AGE_OK) {
        fprintf(stderr, "Round‑trip failed!\n");
        return 1;
    }
    if (memcmp(public_key, decoded_pub, AGE_KEY_BYTES) != 0 ||
        memcmp(secret_key, decoded_sec, AGE_KEY_BYTES) != 0) {
        fprintf(stderr, "Round‑trip mismatch!\n");
        return 1;
    }
    printf("Round‑trip verification passed.\n");

    // 4. Derive public key from secret key
    uint8_t derived_pub[AGE_KEY_BYTES];
    err = age_public_key_from_secret_key(secret_key, derived_pub);
    if (err != AGE_OK) {
        fprintf(stderr, "Derivation failed: %s\n", age_error_string(err));
        return 1;
    }
    if (memcmp(public_key, derived_pub, AGE_KEY_BYTES) == 0) {
        printf("Public key derivation consistent.\n");
    }

    return 0;
}
```

Compile and run:

```bash
gcc -o keygen_example keygen_example.c -lcagekeygen
./keygen_example
```

---

## Buffer Size Rules

| Buffer type                | Use this macro            | Required minimum size      |
| -------------------------- | ------------------------- | -------------------------- |
| Raw key (public or secret) | `AGE_KEY_BYTES`           | 32 bytes                   |
| Public key string          | `AGE_PUBLIC_KEY_BUF_SIZE` | 57 bytes (56 chars + null) |
| Secret key string          | `AGE_SECRET_KEY_BUF_SIZE` | 68 bytes (67 chars + null) |

Passing a buffer smaller than the required size results in undefined
behaviour. The `static` keyword in the function prototypes helps the
compiler catch obvious mismatches, but it is not a runtime check.

---

## Bech32 Alphabet

The encoding uses the following 32‑character Bech32 alphabet (without
checksum, as per the age specification):

```
qpzry9x8gf2tvdw0s3jn54khce6mua7l
```

Characters are case‑sensitive. Uppercase letters are not valid. The library
will reject any string containing characters outside this set.

---

## About Clamping

X25519 requires the secret scalar to be "clamped" before use (clearing
certain bits and setting the high bit). libcagekeygen does **not** clamp the
raw secret key you obtain from `age_generate_keypair()`. Instead, the
underlying Donna implementation performs clamping internally during scalar
multiplication. This means:

- The raw 32‑byte value you get is suitable for storage or export.
- You can give the same raw key to another X25519 library that also clamps
  internally, and the derived public key will match.
- You should **never** manually clamp the key before passing it back to
  libcagekeygen – that would double‑clamp and produce incorrect results.

---

## Security Considerations

- **Secret key storage**: The raw secret key or its string representation
  gives full access to any files encrypted to the corresponding public key.
  Store it with the same care as a password – restricted file permissions
  (`0600`), encrypted storage, or hardware token.
- **Memory handling**: The library does not wipe intermediate values from
  the stack. If you need to erase a secret key after use, use
  `memset_s` or a similar secure‑zero function.
- **Randomness**: The library relies on the operating system's CSPRNG.
  Ensure your system has adequate entropy (e.g., do not use on a
  just‑booted embedded device without a hardware RNG).
- **Thread safety**: All functions are reentrant and can be called from
  multiple threads simultaneously, provided no thread writes to the same
  output buffer without synchronisation.

---

## Compatibility with Age

Keys generated by libcagekeygen are fully compatible with the official
[age](https://github.com/FiloSottile/age) tool. You can:

- Use a public key string produced by this library with `age -r <string>`.
- Save a secret key string to a file and use it with `age -i` (the file
  format is one line per key, no extra spaces).
- Mix keys generated by this library and the official age-keygen without
  any difference.

---

## See Also

- [Quick Start](quick_start.html) – A complete example from scratch.
- [Usage Guide](usage.html) – Integration patterns and best practices.
- [Installation](installation.html) – How to build and install the library.
