---
title: "Installation"
desc: "How to build and install libcagekeygen on Linux, macOS, BSD, and Windows."
---

# Installation

libcagekeygen is written in standard C11 and builds on virtually any platform
with a C compiler. Choose the method that best fits your workflow.

## Prerequisites

- A C compiler (GCC, Clang, MSVC, …)
- **Make** – for the provided `Makefile` (recommended)
- **CMake** (≥ 3.12) – if you prefer CMake or need to build on Windows
- **Git** – to clone the repository

No external libraries are required beyond your system’s libc. On Windows,
the build automatically links against `bcrypt` (provided by the OS).

## Clone the repository

```bash
git clone https://github.com/mroczect/libcagekeygen.git
cd libcagekeygen
```

## Build with Make (recommended for Unix)

The included `Makefile` produces a static library by default.

```bash
make
```

After a successful build you will have:

- `libcagekeygen.a` – static library
- object files inside the source tree (or use `make clean` to remove them)

### Build a shared library

```bash
make shared
```

This creates `libcagekeygen.so` (Linux) or `libcagekeygen.dylib` (macOS).

### Build type

Override the `BUILD_TYPE` variable for debug builds:

```bash
make BUILD_TYPE=Debug
```

Available types: `Release` (default), `Debug`, `MinSizeRel`, `RelWithDebInfo`.

## Build with CMake

CMake is useful for integration with IDEs or for Windows builds.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build
```

The library will be placed inside the `build/` directory.

### CMake options

| Option              | Default | Description                              |
| ------------------- | ------- | ---------------------------------------- |
| `BUILD_SHARED_LIBS` | `OFF`   | Build a shared library instead of static |
| `BUILD_TESTING`     | `OFF`   | Build the test suite                     |
| `ENABLE_SANITIZERS` | `OFF`   | Compile with AddressSanitizer and UBSan  |

Example with tests and sanitizers:

```bash
cmake -B build_test -DBUILD_TESTING=ON -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug .
cmake --build build_test
```

## Run the tests

With **Make**:

```bash
make test
```

With **CMake** (after configuring with `-DBUILD_TESTING=ON`):

```bash
cd build_test
ctest --output-on-failure
```

The test suite includes deterministic unit tests and 500 randomised
round‑trip iterations.

## Install system‑wide

### Using Make

```bash
make install
```

This installs the static library and header to `/usr/local`. You can
change the destination with `PREFIX`:

```bash
make install PREFIX=/usr
```

For staged installations (e.g., packaging):

```bash
make DESTDIR=/tmp/staging install
```

### Using CMake

```bash
cmake --install build
```

The installation prefix can be set at configure time:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr .
cmake --build build
cmake --install build
```

## Uninstall

### With Make

```bash
make uninstall
```

Use the same `PREFIX` that was used during installation:

```bash
make uninstall PREFIX=/opt/mylibs
```

### With CMake

There is no built‑in uninstall target. Remove the installed files manually:

```bash
rm /usr/local/lib/libcagekeygen.a
rm /usr/local/include/libcagekeygen.h
```

## Quick install with the provided script (experimental)

An `installer.sh` script is included in the repository. It downloads a
pre‑built binary release from GitHub and installs it in one step.

**Note:** Pre‑built releases are not yet published for libcagekeygen.
The script is provided as a convenience for future releases and may be
adapted for your own distribution.

```bash
./installer.sh
```

Options:

```
--tag <release_tag>    Install a specific version (e.g., v0.1.0)
--dir <install_dir>    Installation directory (default: /usr/local/bin)
--debug                Verbose output
--help                 Show help
```

## Linking against the library

Once installed, compile your application with:

```bash
gcc -o myapp myapp.c -lcagekeygen
```

If the library is in a non‑standard location, specify the paths:

```bash
gcc -I/path/to/include -L/path/to/lib -o myapp myapp.c -lcagekeygen
```

On Windows, link against `bcrypt` is handled automatically when using
CMake or the provided Makefile. If you are using a different build system,
add `-lbcrypt` explicitly.

## Troubleshooting

**`make: command not found`**  
Install GNU Make (`apt install make`, `brew install make`, etc.).

**`fatal error: libcagekeygen.h: No such file or directory`**  
Ensure the include path is correct (`-I./include` when building from the
source tree, or `-I/usr/local/include` after installation).

**`undefined reference to BCryptGenRandom`** (Windows)  
Add `-lbcrypt` to your linker flags.

**Tests fail with a sanitizer error**  
This usually indicates a bug. Please open an issue on GitHub with the
full sanitizer output.

---

After installation, proceed to the [Quick Start](quick_start.html) guide
for a hands‑on example.
