#!/usr/bin/env bash
# ============================================================================
# Aurora run script (Unix) — launch the AzerothCore WorldServer for Aurora
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

WORLD="${BUILD}/bin/worldserver"

if [[ ! -x "${WORLD}" ]]; then
    echo "WorldServer binary not found: ${WORLD}"
    echo "Build first with scripts/build_linux.sh"
    exit 1
fi

echo "=== Aurora: launching WorldServer (config=${CONFIG}) ==="
exec "${WORLD}" --config="${CONFIG}"
