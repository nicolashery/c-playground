#!/usr/bin/env bash
set -euo pipefail

version() {
    "$@" --version | sed -nE '1s/.*version ([0-9]+(\.[0-9]+)*).*/\1/p'
}

printf '%-22s %s\n' \
    "macOS" "$(sw_vers -productVersion)" \
    "Architecture" "$(uname -m)" \
    "Clang" "$(version clang)" \
    "clang-tidy" "$(version clang-tidy)" \
    "clang-format" "$(version clang-format)" \
    "clangd" "$(version clangd)" \
    "CMake" "$(version cmake)" \
    "Command Line Tools" "$(pkgutil --pkg-info=com.apple.pkg.CLTools_Executables | awk '/^version:/ {print $2}')" \
    "Apple ld" "$(xcrun ld -version_details | sed -nE 's/^[[:space:]]*"version": "([^"]+)".*/\1/p' | head -n 1)" \
    "macOS SDK" "$(xcrun --show-sdk-version)"
