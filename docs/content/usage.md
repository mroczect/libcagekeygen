---
title: "Usage Guide"
desc: "Comprehensive guide to integrating libcagekeygen into real C projects. Covers linking, error handling, buffer management, threading, platform specifics, and common usage patterns."
---

# Usage Guide

This guide describes how to use **libcagekeygen** effectively in your own
applications. It covers compiler and linker settings, all public functions
with detailed examples, error handling, thread safety, platform differences,
and best practices.

---

## 1. Including the header

All public types, constants, and function prototypes are defined in
`libcagekeygen.h`. Include it in every source file that uses the library:

```c
#include "libcagekeygen.h"
```

The header is C++‑compatible – it wraps everything in `extern "C"` when
compiled as C++.

---

## 2. Linking against the library

### Static vs shared linking

- **Static library (`libcagekeygen.a`)** – the library code is embedded into
  your binary. No runtime dependency, but slightly larger binary.
- **Shared library (`libcagekeygen.so` / `.dylib` / `.dll`)** – a separate
  file required at runtime. Multiple programs can share one copy.

The default `make` target builds a static library. Use `make shared` for a
shared build.

### Compiler flags

Basic compilation and linking (static, library in current directory):

```bash
gcc -I./include myapp.c -L. -lcagekeygen -o myapp
```

If the library is installed system‑wide:

```bash
gcc myapp.c -lcagekeygen -o myapp
```

For a custom install prefix (e.g., `/opt/mylibs`):

```bash
gcc -I/opt/mylibs/include myapp.c -L/opt/mylibs/lib -lcagekeygen -o myapp
```

### Windows (MSVC)

When using CMake, linkage to `bcrypt` is automatic. With other build
systems, add `bcrypt.lib` manually.

---

## 3. Buffer size constants

The library never allocates memory. You must provide buffers of the correct
size. **Always use the provided macros** – do not hard‑code numbers.

| Macro                          | Value | Meaning                                                  |
| ------------------------------ | ----- | -------------------------------------------------------- |
| `AGE_KEY_BYTES`                | 32    | Size of a raw key (public or secret) in bytes            |
| `AGE_PUBLIC_KEY_STRING_LENGTH` | 56    | Length of a public key string, excluding null terminator |
| `AGE_SECRET_KEY_STRING_LENGTH` | 67    | Length of a secret key string, excluding null terminator |
| `AGE_PUBLIC_KEY_BUF_SIZE`      | 57    | Minimum buffer size for a public key string (`LENGTH+1`) |
| `AGE_SECRET_KEY_BUF_SIZE`      | 68    | Minimum buffer size for a secret key string (`LENGTH+1`) |

Example declarations:

```c
uint8_t pk[AGE_KEY_BYTES];
uint8_t sk[AGE_KEY_BYTES];
char    pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
char    sec_str[AGE_SECRET_KEY_BUF_SIZE];
```

Passing smaller buffers leads to undefined behaviour. The `static` keyword
in function parameters (e.g., `uint8_t buf[static AGE_KEY_BYTES]`) helps the
compiler warn about obvious mistakes, but it is not a runtime check.

---

## 4. Error handling

Every public function returns an `age_error_t`. **Always check the return
value** before using the output buffers. The possible error codes are:

| Error code                 | Value | Meaning                                                      |
| -------------------------- | ----- | ------------------------------------------------------------ |
| `AGE_OK`                   | 0     | Success                                                      |
| `AGE_ERR_NULL_POINTER`     | -1    | A required pointer was `NULL` (reserved, currently not used) |
| `AGE_ERR_RANDOM_FAILED`    | -2    | System random source could not provide entropy               |
| `AGE_ERR_BUFFER_TOO_SMALL` | -3    | Output buffer too small (reserved, not actively used)        |
| `AGE_ERR_INVALID_FORMAT`   | -4    | Input string does not match the expected age format          |
| `AGE_ERR_KEYGEN_FAILED`    | -5    | Key generation produced a weak (all‑zero) public key         |
| `AGE_ERR_WEAK_PUBLIC_KEY`  | -6    | Decoded public key is the forbidden all‑zero value           |

Obtain a human‑readable description with `age_error_string()`:

```c
const char *desc = age_error_string(err);
// desc is a static string, do not free it.
```

### Error handling pattern

```c
age_error_t err = age_generate_keypair(pk, sk);
if (err != AGE_OK) {
    fprintf(stderr, "Failed to generate keys: %s\n", age_error_string(err));
    return 1;
}
```

Functions that encode keys to strings (`age_public_key_to_string` and
`age_secret_key_to_string`) currently always return `AGE_OK`. However,
checking the return value is still recommended for future compatibility.

---

## 5. Generating a key pair

```c
age_error_t age_generate_keypair(
    uint8_t public_key[static AGE_KEY_BYTES],
    uint8_t secret_key[static AGE_KEY_BYTES]
);
```

This function:

1. Fills `secret_key` with 32 bytes of cryptographically secure random data
   from the operating system.
2. Computes the corresponding public key using the X25519 function:
   `public_key = X25519(secret_key, basepoint)` where `basepoint = 9`.
3. Checks that the public key is not all‑zero (a 2⁻²⁵⁶ probability, which
   would indicate a degenerate secret key).

If the random source fails, it returns `AGE_ERR_RANDOM_FAILED`. If the
public key turns out all‑zero, it returns `AGE_ERR_KEYGEN_FAILED` (the
caller can retry; a fresh random secret will almost certainly succeed).

**Important:** The raw secret key is **not** clamped by this function.
Clamping is performed internally by the Donna implementation during the
scalar multiplication. You can safely pass the raw bytes to other X25519
libraries that also clamp internally.

### Example

```c
uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
age_error_t err = age_generate_keypair(pk, sk);
if (err != AGE_OK) {
    fprintf(stderr, "Error: %s\n", age_error_string(err));
    return 1;
}
```

---

## 6. Deriving a public key from an existing secret key

```c
age_error_t age_public_key_from_secret_key(
    const uint8_t secret_key[static AGE_KEY_BYTES],
    uint8_t public_key[static AGE_KEY_BYTES]
);
```

Use this function when you already have a secret key (e.g., loaded from
a file) and want to recover or display the corresponding public key
without generating a new pair.

It performs the same scalar multiplication as key generation. If the
resulting public key is all‑zero, it returns `AGE_ERR_KEYGEN_FAILED` –
this typically means the secret key was invalid (e.g., all zeros).

```c
uint8_t pk[AGE_KEY_BYTES];
age_error_t err = age_public_key_from_secret_key(sk, pk);
if (err == AGE_ERR_KEYGEN_FAILED) {
    fprintf(stderr, "Secret key is weak (all‑zero or equivalent).\n");
}
```

---

## 7. Encoding keys to age strings

### Public key to string

```c
age_error_t age_public_key_to_string(
    const uint8_t public_key[static AGE_KEY_BYTES],
    char buf[static AGE_PUBLIC_KEY_BUF_SIZE]
);
```

Writes a null‑terminated string of the form `age1` + 52 Bech32 characters.
The output buffer must be at least `AGE_PUBLIC_KEY_BUF_SIZE` (57) bytes.

```c
char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
age_public_key_to_string(pk, pub_str);
printf("%s\n", pub_str);   // e.g., "age1..."
```

### Secret key to string

```c
age_error_t age_secret_key_to_string(
    const uint8_t secret_key[static AGE_KEY_BYTES],
    char buf[static AGE_SECRET_KEY_BUF_SIZE]
);
```

Writes a null‑terminated string of the form `AGE-SECRET-KEY-1` + 52 Bech32
characters. The output buffer must be at least `AGE_SECRET_KEY_BUF_SIZE`
(68) bytes.

```c
char sec_str[AGE_SECRET_KEY_BUF_SIZE];
age_secret_key_to_string(sk, sec_str);
printf("%s\n", sec_str);   // e.g., "AGE-SECRET-KEY-1..."
```

Both functions currently always return `AGE_OK`.

---

## 8. Decoding age strings back to raw keys

### Public key string to raw key

```c
age_error_t age_string_to_public_key(
    const char str[static AGE_PUBLIC_KEY_STRING_LENGTH + 1],
    uint8_t public_key[static AGE_KEY_BYTES]
);
```

Parses a public key string and validates:

1. Exact length (56 printable characters).
2. Prefix `age1`.
3. All remaining 52 characters belong to the Bech32 alphabet
   (`qpzry9x8gf2tvdw0s3jn54khce6mua7l`).
4. The decoded raw key is **not** all zeros.

If any check fails, the function returns `AGE_ERR_INVALID_FORMAT` (or
`AGE_ERR_WEAK_PUBLIC_KEY` for an all‑zero key).

```c
uint8_t pk[AGE_KEY_BYTES];
age_error_t err = age_string_to_public_key("age1...", pk);
if (err == AGE_ERR_INVALID_FORMAT) {
    // malformed string
} else if (err == AGE_ERR_WEAK_PUBLIC_KEY) {
    // all‑zero key
}
```

### Secret key string to raw key

```c
age_error_t age_string_to_secret_key(
    const char str[static AGE_SECRET_KEY_STRING_LENGTH + 1],
    uint8_t secret_key[static AGE_KEY_BYTES]
);
```

Validates:

1. Exact length (67 printable characters).
2. Prefix `AGE-SECRET-KEY-1`.
3. Valid Bech32 characters.

It does **not** reject all‑zero secret keys – an all‑zero secret key
is considered a valid encoding, even though it is cryptographically
unusable. If your application needs to guard against weak secret keys,
check for all‑zero manually after decoding.

```c
uint8_t sk[AGE_KEY_BYTES];
age_error_t err = age_string_to_secret_key("AGE-SECRET-KEY-1...", sk);
if (err == AGE_OK) {
    // optional: check for weak key
    int weak = 1;
    for (int i = 0; i < AGE_KEY_BYTES; i++)
        if (sk[i] != 0) { weak = 0; break; }
    if (weak) fprintf(stderr, "Warning: secret key is all zeros.\n");
}
```

---

## 9. Thread safety

All functions are **reentrant** and do not use any global mutable state.
You can call them from multiple threads simultaneously, provided that:

- No two threads use the same output buffer at the same time without
  synchronisation.
- The system’s random number generator is thread‑safe, which is true for
  all modern platforms supported by the library.

Thus, no additional locking is required in typical usage.

---

## 10. Platform‑specific behaviour

### Linux

Randomness is obtained via `getrandom()` (syscall available since kernel
3.17). No extra libraries needed.

### macOS / BSD

Randomness is obtained by reading `/dev/urandom`. The file descriptor is
opened and closed on each call, which is fine for typical key generation
rates. For high‑throughput scenarios, consider caching the file descriptor
if you modify the source.

### Windows

Randomness comes from `BCryptGenRandom()` with
`BCRYPT_USE_SYSTEM_PREFERRED_RNG`. The library must link against `bcrypt`
(the Makefile and CMake handle this automatically). If you use a custom
build system, add `bcrypt.lib`.

---

## 11. Common usage patterns

### Storing and loading keys from files

The library does not include file I/O functions. You can easily write your
own using the provided string encoding/decoding.

**Saving a key pair:**

```c
FILE *f = fopen("my_age_keys.txt", "w");
if (f) {
    fprintf(f, "# public key\n%s\n", pub_str);
    fprintf(f, "# secret key\n%s\n", sec_str);
    fclose(f);
}
```

**Loading a key pair:**

```c
char line[AGE_SECRET_KEY_BUF_SIZE + 2]; // extra for newline
uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];

FILE *f = fopen("my_age_keys.txt", "r");
// skip comment line, read public key string, decode, etc.
// (A robust parser should strip whitespace and validate.)
```

### Using with the `age` command‑line tool

The strings produced by `age_public_key_to_string` and
`age_secret_key_to_string` are directly compatible with the `age` tool.
You can:

- Generate a key pair, save the strings.
- Encrypt a file: `age -r $(cat pubkey.txt) file.txt`
- Decrypt it with: `age -d -i secretkey.txt file.txt.age`

Because libcagekeygen follows the age specification, keys are
interchangeable.

### Handling multiple keys

There is no “context” object – just raw byte arrays. You can manage
multiple keys with arrays of buffers or structs, e.g.:

```c
typedef struct {
    uint8_t pk[AGE_KEY_BYTES];
    uint8_t sk[AGE_KEY_BYTES];
} AgeKey;

AgeKey key1, key2;
age_generate_keypair(key1.pk, key1.sk);
age_generate_keypair(key2.pk, key2.sk);
```

---

## 12. Advanced: verifying key integrity

After loading a secret key from a file, you may want to ensure it matches
the recorded public key. Derive the public key from the secret and compare:

```c
uint8_t derived_pk[AGE_KEY_BYTES];
age_error_t err = age_public_key_from_secret_key(sk, derived_pk);
if (err == AGE_OK && memcmp(known_pk, derived_pk, AGE_KEY_BYTES) == 0) {
    // key pair is consistent
}
```

---

## 13. Performance considerations

Key generation involves one system call for entropy and one X25519 scalar
multiplication. It is fast (sub‑millisecond on modern hardware). If you
need to generate thousands of keys, consider:

- On Linux, `getrandom()` is efficient; no special handling needed.
- On BSD/macOS, each call opens and closes `/dev/urandom`. For bulk
  generation you could modify `secure_random` in `gen.c` to reuse the file
  descriptor, but this is rarely necessary.

---

## 14. Common pitfalls

- **Using wrong buffer sizes** – Always use `AGE_PUBLIC_KEY_BUF_SIZE` and
  `AGE_SECRET_KEY_BUF_SIZE`, not the `_STRING_LENGTH` constants, when
  declaring char arrays. The string lengths do **not** include the null
  terminator.

- **Not checking return values** – Functions like
  `age_string_to_public_key` can fail silently if you ignore the error.
  The output buffer will be in an undefined state.

- **Confusing public and secret key buffers** – Both are 32 bytes, but
  mixing them up (e.g., passing a public key to
  `age_secret_key_to_string`) will produce a syntactically valid but
  nonsensical string.

- **Forgetting to link `bcrypt` on Windows** – The provided Makefile and
  CMake handle this, but if you use another build system, you must add
  `-lbcrypt` explicitly.

---

## 15. Summary

libcagekeygen is designed to be simple and safe:

- **No allocations** – you own all memory.
- **Clear error codes** – every function returns an `age_error_t`.
- **Cross‑platform** – identical API on Linux, macOS, Windows, BSD.
- **Lightweight** – statically linked, no external crypto libraries.

For a complete reference of every constant and function signature, see the
[API Reference](api_reference.html).

If you haven’t yet built the library, refer to the [Installation](installation.html)
guide. A minimal working example is available in the [Quick Start](quick_start.html).
