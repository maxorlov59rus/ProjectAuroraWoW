#!/usr/bin/env bash
# ============================================================================
# Aurora run script (Unix) — launch the AzerothCore servers for Aurora
# ============================================================================

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd .. && pwd)"
BUILD="${ROOT}/build"
CONFIG="${1:-RelWithDebInfo}"

if [[ ! -d "${BUILD}" ]]; then
    echo "No build directory: ${BUILD}"
    echo "Run scripts/build_linux.sh first."
    exit 1
fi

case "${CONFIG}" in
    RelWithDebInfo|Release|Debug|MinSizeRel) ;;
    *) echo "Unknown config: ${CONFIG}"; exit 1;;
esac

AUTH="${BUILD}/bin/authserver"
WORLD="${BUILD}/bin/worldserver"

if [[ ! -x "${AUTH}" ]]; then
    echo "AuthServer binary not found: ${AUTH}"
    echo "Build first with scripts/build_linux.sh"
    exit 1
fi

echo "=== Aurora: launching AuthServer (config=${CONFIG}) ==="
exec "${AUTH}" --config="${CONFIG}"
