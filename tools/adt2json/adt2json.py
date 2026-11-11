#!/usr/bin/env python3
"""
adt2json — конвертер карт WoW 3.3.5a (ADT) в JSON-чанки для UE-клиента Aurora.

Читает только высотную сетку MCNK/MCVT и позицию тайла. Текстуры, вода,
WMO и M2 не переносятся: для отладочной визуализации рельефа в
FExternalAssetSource этого достаточно.

Использование:
    python tools/adt2json/adt2json.py --input <папка с ADT> --output <куда>
        --map CustomMaps --license user-owned

Значение --license должно быть из белого списка, который принимает
ExternalAssetPolicy в UE-клиенте. Иначе конвертер откажется работать:
это механизм, не дающий случайно опубликовать чужие ассеты.
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

# Лицензии, которые принимает ExternalAssetPolicy::IsAllowedLicense.
ALLOWED_LICENSES = {
    "cc0",
    "cc-by",
    "cc-by-sa",
    "proprietary-own",
    "user-owned",
}

# Сетка высот 3.3.5a: 129 x 129 вершин на тайл.
VERTICES_PER_SIDE = 129
VERTEX_COUNT = VERTICES_PER_SIDE * VERTICES_PER_SIDE

# ADT-тайл покрывает 533.3333 ярда.
TILE_SIZE = 533.3333282470703


class AdtError(RuntimeError):
    pass


def read_chunks(data: bytes):
    """Разбивает ADT на блоки (magic, payload) в порядке следования."""
    offset = 0
    while offset + 8 <= len(data):
        magic = data[offset:offset + 4]
        size = struct.unpack_from("<I", data, offset + 4)[0]
        payload_start = offset + 8
        payload_end = payload_start + size
        if payload_end > len(data):
            raise AdtError(
                f"truncated chunk {magic!r} at {offset}: needs {size} bytes"
            )
        yield magic, data[payload_start:payload_end]
        offset = payload_end


def read_mcnk_heights(payload: bytes) -> list[float]:
    """
    Достаёт MCVT из MCNK.

    Внутри MCNK блоки идут подряд, каждый с собственным magic. Первый из них
    обычно MCNR, второй MCVT. Высоты — 129*129 float32, little-endian.
    """
    for magic, body in read_chunks(payload):
        if magic != b"MCVT":
            continue

        expected = VERTEX_COUNT * 4
        if len(body) < expected:
            raise AdtError(
                f"MCVT too small: {len(body)} bytes, expected {expected}"
            )

        return list(struct.unpack_from(f"<{VERTEX_COUNT}f", body, 0))

    raise AdtError("MCVT chunk not found in MCNK")


def tile_name(path: Path) -> tuple[str, int, int]:
    """CustomMaps_25_57.adt -> ("CustomMaps", 25, 57)."""
    stem = path.stem
    parts = stem.rsplit("_", 2)
    if len(parts) != 3:
        raise AdtError(f"unexpected tile name: {path.name}")
    try:
        return parts[0], int(parts[1]), int(parts[2])
    except ValueError as exc:
        raise AdtError(f"unexpected tile name: {path.name}") from exc


def convert_tile(path: Path) -> dict:
    data = path.read_bytes()

    # Проверяем, что это ADT 3.3.5a, а не другой формат.
    header = {}
    for magic, body in read_chunks(data):
        if magic == b"MHDR":
            header["mhdr"] = body
        if magic == b"MCNK":
            heights = read_mcnk_heights(body)
            break
    else:
        raise AdtError(f"{path.name}: no MCNK chunk, not a 3.3.5a ADT")

    if "mhdr" not in header:
        raise AdtError(f"{path.name}: no MHDR chunk")

    return {"heights": heights}


def build_manifest(output: Path, entries: list[dict]) -> dict:
    return {"files": entries}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path,
                        help="folder with CustomMaps_*.adt files")
    parser.add_argument("--output", required=True, type=Path,
                        help="destination folder for JSON chunks")
    parser.add_argument("--map", default=None,
                        help="map name for manifest (default: from file names)")
    parser.add_argument("--license", required=True,
                        help="one of: " + ", ".join(sorted(ALLOWED_LICENSES)))
    parser.add_argument("--base-x", type=float, default=0.0,
                        help="world X of the first tile corner (default 0)")
    parser.add_argument("--base-y", type=float, default=0.0,
                        help="world Y of the first tile corner (default 0)")
    args = parser.parse_args(argv)

    if args.license not in ALLOWED_LICENSES:
        print(
            f"ERROR: license '{args.license}' is not accepted. "
            f"Allowed: {', '.join(sorted(ALLOWED_LICENSES))}",
            file=sys.stderr,
        )
        return 2

    if not args.input.is_dir():
        print(f"ERROR: input folder not found: {args.input}", file=sys.stderr)
        return 2

    tiles = sorted(args.input.glob("*.adt"))
    if not tiles:
        print(f"ERROR: no .adt files in {args.input}", file=sys.stderr)
        return 2

    terrain_dir = args.output / "terrain"
    terrain_dir.mkdir(parents=True, exist_ok=True)

    entries: list[dict] = []
    converted = 0
    failed = 0

    for tile in tiles:
        try:
            map_name, tile_x, tile_y = tile_name(tile)
        except AdtError as exc:
            print(f"  skip {tile.name}: {exc}")
            failed += 1
            continue

        try:
            data = convert_tile(tile)
        except AdtError as exc:
            print(f"  skip {tile.name}: {exc}")
            failed += 1
            continue

        heights = data["heights"]

        # Мировые координаты: тайл смещён на TileX/TileY тайлов от угла.
        # Ось Y в WoW растёт вниз по карте, поэтому берём со знаком минус.
        origin_x = args.base_x + tile_x * TILE_SIZE
        origin_y = args.base_y - tile_y * TILE_SIZE

        # Шаг сетки: 533.33 ярда на 128 интервалов.
        step = TILE_SIZE / (VERTICES_PER_SIDE - 1)

        vertices = []
        index = 0
        for row in range(VERTICES_PER_SIDE):
            for col in range(VERTICES_PER_SIDE):
                vertices.append({
                    "x": round(origin_x + col * step, 3),
                    "y": round(origin_y - row * step, 3),
                    "z": round(heights[index], 3),
                })
                index += 1

        out_name = f"terrain/{tile.stem}.json"
        payload = {
            "rows": VERTICES_PER_SIDE,
            "cols": VERTICES_PER_SIDE,
            "vertices": vertices,
        }
        (args.output / out_name).write_text(
            json.dumps(payload, separators=(",", ":")), encoding="utf-8"
        )

        entries.append({
            "path": out_name,
            "map": args.map or map_name,
            "license": args.license,
        })
        converted += 1
        print(f"  {tile.name} -> {out_name} ({len(vertices)} vertices)")

    if converted == 0:
        print("ERROR: nothing converted", file=sys.stderr)
        return 1

    manifest = build_manifest(args.output, entries)
    (args.output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False), encoding="utf-8"
    )

    print(f"done: {converted} converted, {failed} skipped")
    print(f"manifest: {args.output / 'manifest.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
