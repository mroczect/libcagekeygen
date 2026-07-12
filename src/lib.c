/**
 * @file lib.c
 * @brief Aggregation unit for the libcagekeygen library.
 *
 * This source file exists solely for the convenience of single‑translation‑unit
 * (a.k.a. "unity") builds. It includes the public header `libcagekeygen.h` and
 * does **not** define any additional functions or data.
 *
 * All actual implementations reside in separate modules:
 * - `gen.c`         – key generation and derivation
 * - `io.c`          – string encoding/decoding (Bech32)
 * - `error.c`       – error‑to‑string conversion
 * - `curve25519-donna.c` – low‑level X25519 scalar multiplication
 *
 * When the library is compiled as a single logical unit, all of the above
 * translation units (and this one) are compiled and linked together. The
 * public API is completely described in the header file; there is no private
 * API outside of the static helpers inside each module.
 *
 * @note This file is optional. If you prefer to compile and link each .c file
 *       individually, you may omit `lib.c` from your build system without
 *       losing any functionality.
 *
 * @author mroczect
 * @version 0.0.1
 * @copyright GPLv3
 */

#include "libcagekeygen.h"

/*
 * This file is intentionally left empty of executable code.
 *
 * All library functionality is implemented in the other source files.
 * Including this file in a build simply causes the linker to pull in
 * the compiled objects from the other modules (when built as a single
 * compilation unit or via a properly configured build system).
 *
 * For projects using CMake or Make, this file can be listed alongside
 * the other .c files in the target sources.
 */
