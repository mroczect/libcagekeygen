LIBCAGEKEYGEN
=============

A minimal, dependency‑free C library to generate X25519 key pairs and
convert them to and from the standard age string format (Bech32 encoding).
It produces keys that can be directly used with the age encryption tool.


OVERVIEW
--------

libcagekeygen provides a small set of functions that perform the essential
operations for handling age-compatible keys:

  - Generating cryptographically secure X25519 key pairs.
  - Deriving a public key from an existing secret key.
  - Encoding raw 32‑byte keys into the standard age strings:
      Public key  →  "age1…"
      Secret key  →  "AGE-SECRET-KEY-1…"
  - Decoding those strings back to raw keys with full validation.

The library is written in standard C (C99/C11), has no dependencies beyond
the C standard library and a bundled, well‑known X25519 implementation
(curve25519‑donna).  It does not perform dynamic memory allocation – all
buffers are caller‑provided, and their required sizes are fully defined
by preprocessor macros.


FEATURES
--------

- Single‑purpose API – generate, derive, encode, decode age keys.
- No external dependencies – relies only on the C standard library and
  the included curve25519‑donna implementation.
- Portable – builds on Linux, macOS, Windows, and BSD systems.
  Platform‑specific secure random sources are abstracted internally.
- Binary safe – keys are handled as fixed‑size 32‑byte arrays.
- Full string round‑trip – encode and decode public and secret keys
  without loss.
- Input validation – invalid strings are rejected with clear error codes;
  weak (all‑zero) public keys are forbidden.
- Informative errors – every function returns an `age_error_t` code;
  `age_error_string()` converts it to a human‑readable message.
- C++ compatible – header uses `extern "C"` for seamless use in C++.
- Configurable build – CMake build with optional sanitizers and strict
  warnings.  A convenience Makefile is also provided.
- Tested – includes a suite of deterministic and randomised tests (500
  iterations) runnable with `make test`.


GETTING STARTED
---------------

Prerequisites:

  - A C compiler (GCC, Clang, MSVC, …)
  - CMake (≥ 3.12) – for building
  - GNU Make (or compatible) – for the provided convenience Makefile
  - Standard POSIX tools (ar, install) for installation

Building from Source:

  git clone https://github.com/mroczect/libcagekeygen.git
  cd libcagekeygen
  make

  This produces:
    - libcagekeygen.a   – static library
    - (no command‑line tool is included; the library is meant to be
       linked into your own application)

  Object files are placed in the `build/` directory and are ignored by Git.

Available Make targets:

  all        Build the library (default)
  build      Same as `all`
  clean      Remove all build artifacts (build, build_test, build_asan)
  test       Build and run the test suite (Debug mode)
  asan       Build and run tests with AddressSanitizer and UBSan
  install    Install library and header to /usr/local (requires `build`)

  Override the compiler or flags:

    make CC=clang BUILD_TYPE=Debug

  The install target respects DESTDIR for staged installations:

    make DESTDIR=/tmp/staging install

  CMake can also be used directly:

    cmake -B build -DCMAKE_BUILD_TYPE=Release .
    cmake --build build
    cmake --install build

  To build the tests:

    cmake -B build_test -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug .
    cmake --build build_test
    ctest --test-dir build_test --output-on-failure

Linking Against the Library:

  After building, include the header and link with `-lcagekeygen`.

    #include "libcagekeygen.h"

    int main(void) {
        uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
        age_error_t err = age_generate_keypair(pk, sk);
        if (err) {
            fprintf(stderr, "Error: %s\n", age_error_string(err));
            return 1;
        }
        char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
        age_public_key_to_string(pk, pub_str);
        printf("Public key: %s\n", pub_str);
        return 0;
    }

  Compile and link:

    gcc -I./include myapp.c -L. -lcagekeygen -o myapp

  On Windows, the library automatically links against `bcrypt` (provided
  by the system) – no extra flags are needed when using CMake.


USAGE
-----

There is no standalone command‑line tool in this project.  The library is
intended to be integrated into other programs.  The API reference below
describes every function and constant.


API REFERENCE
-------------

All functions are declared in `include/libcagekeygen.h`.

Constants

  AGE_KEY_BYTES                    32
    Raw key size in bytes (256 bits).

  AGE_PUBLIC_KEY_STRING_LENGTH     56
    Number of printable characters in a public key string ("age1" + 52).

  AGE_SECRET_KEY_STRING_LENGTH     67
    Number of printable characters in a secret key string
    ("AGE-SECRET-KEY-1" + 52).

  AGE_PUBLIC_KEY_BUF_SIZE          57
    Minimum buffer size (including null terminator) for a public key
    string.

  AGE_SECRET_KEY_BUF_SIZE          68
    Minimum buffer size (including null terminator) for a secret key
    string.

Error codes (age_error_t)

  AGE_OK                    0   Success.
  AGE_ERR_NULL_POINTER     -1   NULL pointer argument (reserved).
  AGE_ERR_RANDOM_FAILED    -2   Failed to obtain secure random bytes.
  AGE_ERR_BUFFER_TOO_SMALL -3   Output buffer too small (reserved).
  AGE_ERR_INVALID_FORMAT   -4   Input string does not match the age format.
  AGE_ERR_KEYGEN_FAILED    -5   Key generation produced a weak (all-zero)
                                public key.
  AGE_ERR_WEAK_PUBLIC_KEY  -6   Decoded public key is the forbidden
                                all-zero value.

Use `age_error_string(err)` to obtain a static description string.

Functions

  age_generate_keypair()

    age_error_t age_generate_keypair(
        uint8_t public_key[static AGE_KEY_BYTES],
        uint8_t secret_key[static AGE_KEY_BYTES]);

    Fills `secret_key` with 32 random bytes and computes the corresponding
    public key.  Returns `AGE_ERR_RANDOM_FAILED` if the random source
    fails, or `AGE_ERR_KEYGEN_FAILED` if the resulting public key is
    all zeros (extremely unlikely).

  age_public_key_from_secret_key()

    age_error_t age_public_key_from_secret_key(
        const uint8_t secret_key[static AGE_KEY_BYTES],
        uint8_t public_key[static AGE_KEY_BYTES]);

    Derives the public key from an existing secret key.  Returns
    `AGE_ERR_KEYGEN_FAILED` if the derived public key is all zeros.

  age_public_key_to_string()

    age_error_t age_public_key_to_string(
        const uint8_t public_key[static AGE_KEY_BYTES],
        char buf[static AGE_PUBLIC_KEY_BUF_SIZE]);

    Encodes a raw public key into the age string format (`"age1"` + 52
    Bech32 characters).  Always returns `AGE_OK`.

  age_secret_key_to_string()

    age_error_t age_secret_key_to_string(
        const uint8_t secret_key[static AGE_KEY_BYTES],
        char buf[static AGE_SECRET_KEY_BUF_SIZE]);

    Encodes a raw secret key into `"AGE-SECRET-KEY-1"` + 52 Bech32
    characters.  Always returns `AGE_OK`.

  age_string_to_public_key()

    age_error_t age_string_to_public_key(
        const char str[static AGE_PUBLIC_KEY_STRING_LENGTH + 1],
        uint8_t public_key[static AGE_KEY_BYTES]);

    Decodes a public key string back to 32 bytes.  Validates length,
    prefix, character set, and rejects all-zero keys.  Returns
    `AGE_ERR_INVALID_FORMAT` or `AGE_ERR_WEAK_PUBLIC_KEY` on failure.

  age_string_to_secret_key()

    age_error_t age_string_to_secret_key(
        const char str[static AGE_SECRET_KEY_STRING_LENGTH + 1],
        uint8_t secret_key[static AGE_KEY_BYTES]);

    Decodes a secret key string.  Validates format but does not reject
    all-zero keys.  Returns `AGE_ERR_INVALID_FORMAT` on error.

  age_error_string()

    const char *age_error_string(age_error_t err);

    Returns a static English description of the error code.
    Never returns NULL; unknown codes yield "unknown error".


PROJECT STRUCTURE
-----------------

  libcagekeygen/
  ├── include/
  │   └── libcagekeygen.h        Public header
  ├── src/
  │   ├── curve25519-donna.c     X25519 implementation (Donna)
  │   ├── gen.c                  Key generation & derivation
  │   ├── io.c                   Bech32 encoding/decoding
  │   ├── error.c                Error string function
  │   └── lib.c                  Unity‑build aggregation (optional)
  ├── test/
  │   ├── CMakeLists.txt
  │   ├── test.c                 Test runner
  │   ├── test_gen.c             Generation tests
  │   ├── test_io.c              I/O tests
  │   ├── test_random.c          Randomised round‑trip tests
  │   └── test_utils.h           Minimal test framework
  ├── CMakeLists.txt             Root build configuration
  ├── Makefile                   Convenience targets
  ├── LICENSE                    GNU GPLv3
  └── README.txt                 This file


VERSIONING
----------

This project follows Semantic Versioning (https://semver.org).
The current version is 0.0.1.


CONTRIBUTING
------------

Contributions are welcome.  Please open an issue or submit a pull request
on GitHub.  Ensure that:

  - Code compiles cleanly with `-Wall -Wextra -Wpedantic -Werror`.
  - Existing functionality is not broken.
  - New tests are added if applicable.


LICENSE
-------

libcagekeygen

Copyright (C) 2026 mroczect

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

The library includes curve25519‑donna, which is covered by a separate
BSD‑style license.  See the header of `src/curve25519-donna.c` for details.


CONTACT
-------

Repository: https://github.com/mroczect/libcagekeygen