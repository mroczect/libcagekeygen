#!/usr/bin/env bash
#
# uninstall.sh – Remove libcagekeygen from the system
#
# Usage:
#   ./uninstall.sh [prefix]
#
# If no prefix is given, defaults to /usr/local.
# Uses sudo automatically if the directory is not writable.

set -euo pipefail

PREFIX="${1:-/usr/local}"

echo "Removing libcagekeygen from $PREFIX ..."

if [ -w "$PREFIX" ]; then
    make uninstall PREFIX="$PREFIX"
else
    sudo make uninstall PREFIX="$PREFIX"
fi

echo "Uninstall complete."
