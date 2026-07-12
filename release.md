# libcagekeygen

<!--
  RELEASE TEMPLATE
  Before tagging a new version, replace the heading above with:
  # libcagekeygen vX.Y.Z – Short release title
  Then commit the change together with the tag.
-->

## What's included

- Generate secure X25519 key pairs (`age_generate_keypair`)
- Derive a public key from an existing secret key (`age_public_key_from_secret_key`)
- Encode raw keys to standard age strings:
  - Public key  → `age1...`
  - Secret key  → `AGE-SECRET-KEY-1...`
- Decode age strings back to raw keys with full validation
- Human‑readable error messages via `age_error_string()`
- All operations use caller‑provided buffers; no dynamic memory
- Portable C11 code, builds on Linux, macOS, Windows, and BSD
- Bundled `curve25519-donna` implementation (no external crypto libraries)
- Test suite with deterministic and randomised tests (500 iterations)
- Simple Makefile and CMake build options

## Pre‑built archives

Each ZIP file contains:
- `libcagekeygen.a` – static library
- `libcagekeygen.h` – public header

## Installation

### From pre‑built archives
```
unzip libcagekeygen-<platform>.zip
gcc -I. myapp.c -L. -lcagekeygen -o myapp
```

### From source
```
git clone https://github.com/mroczect/libcagekeygen.git
cd libcagekeygen
make
make install
```

## SHA256 checksums

Verify the integrity of the downloaded archives using the provided
`SHA256SUMS` file:
```
sha256sum -c SHA256SUMS
```

## Documentation

See the [README.txt](https://github.com/mroczect/libcagekeygen/blob/master/README.txt)
for the full API reference and build instructions.

## License

GNU General Public License v3.0.  
The included `curve25519-donna` code is covered by a separate BSD‑style
license (see its source header).