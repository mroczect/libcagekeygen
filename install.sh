#!/usr/bin/env bash
#
# install.sh – Quick installer for libcagekeygen
#
# Builds and installs the library in a single step.
# Default: static library, prefix /usr/local.
#
# Usage:
#   ./install.sh [--prefix=/path] [--shared] [--no-build] [--help]
#
# Options:
#   --prefix=/path   Installation directory (default: /usr/local)
#   --shared          Build and install the shared library instead of static
#   --no-build        Skip compilation, only install (assumes already built)
#   --help            Print this help and exit

set -euo pipefail

PREFIX="/usr/local"
BUILD=true
SHARED=false

show_help() {
    grep '^#' "$0" | grep -v '^#!' | sed 's/^# \?//'
    exit 0
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix=*)    PREFIX="${1#*=}" ;;
        --shared)      SHARED=true ;;
        --no-build)    BUILD=false ;;
        --help)        show_help ;;
        *)             echo "Unknown option: $1"; show_help ;;
    esac
    shift
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "libcagekeygen installer"
echo ""
echo "  Prefix : $PREFIX"
echo "  Type   : $([ "$SHARED" = true ] && echo "shared" || echo "static")"
echo ""

if $BUILD; then
    echo "[1/2] Building library..."
    if $SHARED; then
        make shared PREFIX="$PREFIX"
    else
        make static PREFIX="$PREFIX"
    fi
    echo ""
fi

echo "[2/2] Installing to $PREFIX ..."
if [ -w "$PREFIX" ]; then
    if $SHARED; then
        make install-shared PREFIX="$PREFIX"
    else
        make install PREFIX="$PREFIX"
    fi
else
    echo "  (requesting sudo for installation)"
    if $SHARED; then
        sudo make install-shared PREFIX="$PREFIX"
    else
        sudo make install PREFIX="$PREFIX"
    fi
fi

echo ""
echo "libcagekeygen installed successfully."
echo ""
echo "Usage example:"
echo '    gcc -o myapp myapp.c -lcagekeygen'
