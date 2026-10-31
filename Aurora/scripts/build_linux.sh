#!/usr/bin/env bash
# ============================================================================
# Aurora build script (Linux)
# Builds the AzerothCore-based server with the Aurora custom layer.
# ============================================================================

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd .. && pwd)"
BUILD="${ROOT}/build"
CONFIG="RelWithDebInfo"
GEN="Ninja"
MSG="${MSG_COLOR:=}"

info()  { echo -e "  [i] $*"; }
warn()  { echo -e "  [!] $*"; }
error() { echo -e "  [E] $*"; }

# --- argument parsing ------------------------------------------------------
usage() {
    echo "Usage: $0 [--debug|--release] [--vs2022|--ninja]"
    echo "  --release      Build type: Release (default RelWithDebInfo)"
    echo "  --vs2022       Generator: Visual Studio 2022 (Windows only, ignored on Linux)"
    echo "  --ninja        Generator: Ninja (Unix Makefile fallback)"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --release)   CONFIG="Release"; shift;;
        --debug)     CONFIG="Debug"; shift;;
        --vs2022)    GEN="Visual Studio 17 2022"; shift;;
        --ninja)     GEN="Ninja"; shift;;
        -h|--help)   usage;;
        *)           error "Unknown argument: $1"; usage;;
    esac
done

echo
echo "  === Aurora build (Linux) ==="
echo "  Root:   $ROOT"
echo "  Build:  $BUILD"
echo "  Config: $CONFIG"
echo "  Gen:    $GEN"
echo

# --- check tools ---------------------------------------------------------
command -v cmake >/dev/null 2>&1 || { error "cmake not found in PATH."; exit 1; }
command -v make   >/dev/null 2>&1 || { error "make not found in PATH."; exit 1; }

# --- configure -------------------------------------------------------------
echo "[1/3] Configuring..."
cmake -S "$ROOT" -B "$BUILD" -G "$GEN" \
      -DCMAKE_BUILD_TYPE="$CONFIG" \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DCMAKE_CXX_EXTENSIONS=OFF
info "Configure done."

# --- build -----------------------------------------------------------------
echo "[2/3] Building..."
cmake --build "$BUILD" --config "$CONFIG"
info "Build done."

# --- summary ---------------------------------------------------------------
echo
echo "[3/3] Build finished."
echo "  Binaries:"
echo "    - WorldServer: $BUILD/bin/worldserver"
echo "    - AuthServer:  $BUILD/bin/authserver"
echo
echo "  Run:"
echo "    $BUILD/bin/authserver"
echo "    $BUILD/bin/worldserver"
echo
