---
title: "Quick Start – Complete Guide"
desc: "A step‑by‑step guide to generating, encoding, decoding, and verifying age‑compatible keys with libcagekeygen, from cloning the repository to running your first program."
---

# Quick Start – Complete Guide

This guide takes you from a clean system to a fully functional program that
uses **libcagekeygen** to create, display, and verify age encryption keys.
No prior knowledge of the library is assumed.

---

## 1. Prerequisites

You need:

- A C compiler (GCC, Clang, or MSVC)
- GNU Make (for the Makefile) or CMake (≥ 3.12)
- Git (to clone the repository)
- Basic familiarity with the command line

The library has **no external dependencies** – it bundles everything needed.

---

## 2. Clone the repository

```bash
git clone https://github.com/mroczect/libcagekeygen.git
cd libcagekeygen
```

---

## 3. Build the library

### Option A – Using Make (recommended)

Build the static library:

```bash
make
```

This creates `libcagekeygen.a` and object files.  
To build a shared library instead:

```bash
make shared
```

### Option B – Using CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build
```

The library will be inside `build/`.

### Verify the build

You should see the library file:

```bash
ls -l libcagekeygen.a          # Make
# or
ls -l build/libcagekeygen.a    # CMake
```

---

## 4. Write your first program

Create a file named `example.c` with the following content.  
We will write a program that:

1. Generates a fresh X25519 key pair.
2. Prints the public and secret keys in the standard `age` format.
3. Decodes the public key back to raw bytes.
4. Verifies that the decoded key matches the original.
5. Demonstrates error handling.

```c
/**
 * example.c – Complete libcagekeygen demonstration
 *
 * Compile (from the project root):
 *   gcc -I./include example.c -L. -lcagekeygen -o example
 */

#include <stdio.h>
#include <string.h>
#include "libcagekeygen.h"

int main(void) {
    /* ---------------------------------------------------------
     * 1. Declare buffers – sizes are defined by the library
     * --------------------------------------------------------- */
    uint8_t pk[AGE_KEY_BYTES];                     // 32 bytes
    uint8_t sk[AGE_KEY_BYTES];                     // 32 bytes
    char    pub_str[AGE_PUBLIC_KEY_BUF_SIZE];       // 57 bytes (56 chars + null)
    char    sec_str[AGE_SECRET_KEY_BUF_SIZE];       // 68 bytes (67 chars + null)
    age_error_t err;

    /* ---------------------------------------------------------
     * 2. Generate a fresh key pair
     * --------------------------------------------------------- */
    err = age_generate_keypair(pk, sk);
    if (err != AGE_OK) {
        fprintf(stderr, "Key generation failed: %s\n", age_error_string(err));
        return 1;
    }

    /* ---------------------------------------------------------
     * 3. Encode keys to age strings
     * --------------------------------------------------------- */
    age_public_key_to_string(pk, pub_str);
    age_secret_key_to_string(sk, sec_str);

    printf("=== Generated Keys ===\n");
    printf("Public key : %s\n", pub_str);
    printf("Secret key : %s\n\n", sec_str);

    /* ---------------------------------------------------------
     * 4. Decode the public key and verify round‑trip
     * --------------------------------------------------------- */
    uint8_t pk2[AGE_KEY_BYTES];
    err = age_string_to_public_key(pub_str, pk2);
    if (err != AGE_OK) {
        fprintf(stderr, "Decoding failed: %s\n", age_error_string(err));
        return 1;
    }

    int match = (memcmp(pk, pk2, AGE_KEY_BYTES) == 0);
    printf("=== Verification ===\n");
    printf("Round‑trip (decode → compare) : %s\n", match ? "PASSED" : "FAILED");

    /* ---------------------------------------------------------
     * 5. Derive the public key from the secret key independently
     * --------------------------------------------------------- */
    uint8_t pk3[AGE_KEY_BYTES];
    err = age_public_key_from_secret_key(sk, pk3);
    if (err != AGE_OK) {
        fprintf(stderr, "Derivation failed: %s\n", age_error_string(err));
        return 1;
    }

    int derive_ok = (memcmp(pk, pk3, AGE_KEY_BYTES) == 0);
    printf("Derive public from secret     : %s\n", derive_ok ? "PASSED" : "FAILED");

    /* ---------------------------------------------------------
     * 6. (Optional) Show error string for an invalid operation
     * --------------------------------------------------------- */
    printf("\n=== Error Demo ===\n");
    // Try to decode a malformed string
    const char *bad_input = "age1!!!!";   // wrong length + invalid chars
    err = age_string_to_public_key(bad_input, pk2);
    printf("Decoding invalid string: %s\n", age_error_string(err));

    return 0;
}
```

---

## 5. Compile the example

Make sure you are in the root of the repository (where `libcagekeygen.a` is
located).

```bash
gcc -I./include example.c -L. -lcagekeygen -o example
```

If you built with CMake and the library is inside `build/`:

```bash
gcc -I./include example.c -L./build -lcagekeygen -o example
```

---

## 6. Run the example

```bash
./example
```

You should see output similar to this (the actual keys will differ):

```
=== Generated Keys ===
Public key : age1...
Secret key : AGE-SECRET-KEY-1...

=== Verification ===
Round‑trip (decode → compare) : PASSED
Derive public from secret     : PASSED

=== Error Demo ===
Decoding invalid string: invalid age string format
```

> **Note**: The public key string will always start with `age1` and be
> exactly 56 characters long. The secret key string starts with
> `AGE-SECRET-KEY-1` and is 67 characters long.

---

## 7. Understand what happened

### Key generation

`age_generate_keypair()` fills the `sk` buffer with 32 bytes of
cryptographically secure random data. It then computes the corresponding
public key by performing the X25519 scalar multiplication of `sk` with the
standard curve base point (the number 9).

The raw secret key is **not** clamped by the function – that is done
internally inside `curve25519_donna` during the multiplication. You can use
the raw bytes as‑is with other X25519 libraries that also clamp internally.

### Encoding to strings

`age_public_key_to_string()` and `age_secret_key_to_string()` convert raw
32‑byte keys into the standard `age` string format using a custom Bech32
encoding (without checksum). The strings are null‑terminated and ready for
printing or storage.

### Decoding and validation

`age_string_to_public_key()` reverses the process. It checks:

- The string is exactly 56 characters long.
- It starts with `age1`.
- The remaining 52 characters are valid Bech32 symbols.
- The decoded key is **not** all zeros (a forbidden weak key).

If any check fails, the function returns an appropriate error code.

### Derivation

`age_public_key_from_secret_key()` re‑computes the public key from an
existing secret key. It’s useful when you’ve stored the secret key and want
to recover or display the corresponding public key without generating a new
pair.

---

## 8. Error handling essentials

Every library function returns an `age_error_t`. **Always check the return
value** before using the output buffers.

| Return value                 | Meaning                                           |
|------------------------------|---------------------------------------------------|
| `AGE_OK`                     | Success                                           |
| `AGE_ERR_RANDOM_FAILED`      | System random source failed                       |
| `AGE_ERR_INVALID_FORMAT`     | Input string doesn’t match the expected format    |
| `AGE_ERR_KEYGEN_FAILED`      | Generated public key was all‑zero (extremely rare)|
| `AGE_ERR_WEAK_PUBLIC_KEY`    | Decoded public key is the forbidden all‑zero value|

Use `age_error_string(err)` to obtain a static human‑readable description.

```c
age_error_t err = age_generate_keypair(pk, sk);
if (err != AGE_OK) {
    fprintf(stderr, "Error: %s\n", age_error_string(err));
    return 1;
}
```

---

## 9. Buffer sizes – get them right

Always use the provided macros. Never hardcode numbers.

```c
uint8_t pk[AGE_KEY_BYTES];
char    pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
```

- `AGE_KEY_BYTES` = 32
- `AGE_PUBLIC_KEY_BUF_SIZE` = 57 (`AGE_PUBLIC_KEY_STRING_LENGTH` + 1)
- `AGE_SECRET_KEY_BUF_SIZE` = 68 (`AGE_SECRET_KEY_STRING_LENGTH` + 1)

Passing a buffer that is too small leads to undefined behaviour. The
`static` keyword in the function prototypes helps the compiler warn about
obvious mismatches, but it is not a runtime check.

---

## 10. Next steps

- For a complete reference of all constants, types, and functions, see the
  [API Reference](api_reference.html).
- To learn about platform‑specific behaviour, linking options, and thread
  safety, read the [Usage Guide](usage.html).
- If you haven’t installed the library system‑wide, the
  [Installation](installation.html) page covers all methods (Make, CMake,
  manual, and the quick installer).

---

## 11. Troubleshooting

### Compiler cannot find `libcagekeygen.h`

Make sure you pass `-I./include` (or `-I/path/to/include` after
installation).

### Linker errors about undefined references

- Verify you linked with `-lcagekeygen` **after** your source file(s).
- If using a non‑standard library path, add `-L/path/to/lib`.
- On Windows, ensure `-lbcrypt` is present (the Makefile and CMake handle
  this automatically).

### Program crashes or outputs garbage

Check that you didn’t pass a buffer that is too small. Always use the macros
(`AGE_KEY_BYTES`, `AGE_PUBLIC_KEY_BUF_SIZE`, etc.).

---

**Congratulations!** You’re now ready to use libcagekeygen in your own
projects.
