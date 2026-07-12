---
title: "libcagekeygen"
desc: "A minimal, dependency‑free C library to generate X25519 key pairs and convert them to and from the standard age string format (Bech32 encoding). It produces keys that can be directly used with the age encryption tool."
author: "mroczect"
repo_url: "https://github.com/mroczect/libcagekeygen"
license: "GPL-3.0"
---

# libcagekeygen

**Generate and handle age‑compatible X25519 keys in pure C.**

libcagekeygen is a tiny, self‑contained library that performs the essential operations for working with [age](https://age-encryption.org) keys:

- Create cryptographically secure key pairs.
- Derive public keys from existing secrets.
- Encode raw keys into standard `age1…` and `AGE-SECRET-KEY-1…` strings.
- Decode those strings back to raw bytes with full validation.

No dynamic memory, no external crypto libraries – just your C compiler and the included `curve25519-donna` implementation.

---

## Features

- **Single‑purpose API** – 7 clean functions cover generation, derivation, encoding, decoding, and error reporting.
- **Zero dependencies** – only the C standard library and the bundled `curve25519-donna`.
- **Portable** – builds on Linux, macOS, Windows, BSD. Platform‑specific secure random sources are handled internally.
- **Buffer‑safe** – all output buffers are caller‑allocated; sizes are defined by compile‑time macros.
- **Full validation** – decoders check length, prefix, character set, and reject weak (all‑zero) public keys.
- **Clear errors** – every function returns an `age_error_t`, and `age_error_string()` gives a human‑readable description.
- **C++ compatible** – header uses `extern "C"`.
- **Thoroughly tested** – deterministic unit tests plus 500 randomised round‑trip iterations.

---

## Quick start

Clone the repository and build the library:

```bash
git clone https://github.com/mroczect/libcagekeygen.git
cd libcagekeygen
make
```

This produces `libcagekeygen.a`. Then include the header and link against the library:

```c
#include "libcagekeygen.h"

int main(void) {
    uint8_t pk[AGE_KEY_BYTES], sk[AGE_KEY_BYTES];
    age_error_t err = age_generate_keypair(pk, sk);
    if (err != AGE_OK) {
        fprintf(stderr, "Error: %s\n", age_error_string(err));
        return 1;
    }

    char pub_str[AGE_PUBLIC_KEY_BUF_SIZE];
    age_public_key_to_string(pk, pub_str);
    printf("Public key: %s\n", pub_str);
    return 0;
}
```

Compile and run:

```bash
gcc -I./include myapp.c -L. -lcagekeygen -o myapp
./myapp
```

For more details, see the [Installation](installation.html) and [Quick Start](quick_start.html) guides.

---

## Where to go from here

- [Installation](installation.html) – full build instructions for all platforms.
- [Quick Start](quick_start.html) – step‑by‑step usage example.
- [API Reference](api_reference.html) – detailed description of every constant, function, and error code.
- [Usage Guide](usage.html) – integrating the library into real projects.

---

## Project structure

```
libcagekeygen/
├── include/
│   └── libcagekeygen.h          Public header
├── src/
│   ├── curve25519-donna.c       X25519 implementation
│   ├── gen.c                    Key generation & derivation
│   ├── io.c                     Bech32 encoding/decoding
│   ├── error.c                  Error string function
│   └── lib.c                    Unity‑build aggregator
├── test/                        Unit tests
├── CMakeLists.txt               CMake build
├── Makefile                     Standalone Make build
├── LICENSE                      GPLv3
└── README.txt                   Full project readme
```

---

## License

libcagekeygen is distributed under the GNU General Public License v3.0.  
The included `curve25519-donna` code is covered by its own BSD‑style license. See the source header for details.
