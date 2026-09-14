#!/usr/bin/env python3
"""Pack PNG files into a macOS .icns container, using only the stdlib.

Usage:
    png-to-icns.py OUT.icns SIZE:PATH [SIZE:PATH ...]

The size may also come from a file name like "soundchest-256.png". ImageMagick
on Linux often cannot write ICNS, and iconutil only exists on macOS, so this
keeps icon generation reproducible on any machine.
"""

from __future__ import annotations

import re
import struct
import sys
from pathlib import Path

# Which ICNS chunks each pixel size can fill. The @2x types reuse the same PNGs.
TYPES_FOR_SIZE = {
    16: ("icp4",),
    32: ("icp5", "ic11"),
    64: ("icp6", "ic12"),
    128: ("ic07",),
    256: ("ic08", "ic13"),
    512: ("ic09", "ic14"),
    1024: ("ic10",),
}

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def parse_argument(argument: str) -> tuple[int, Path]:
    if ":" in argument:
        raw_size, raw_path = argument.split(":", 1)
        return int(raw_size), Path(raw_path)

    path = Path(argument)
    match = re.search(r"-(\d+)\.png$", path.name)
    if not match:
        raise SystemExit(f"error: cannot infer a pixel size from {path.name!r}; pass SIZE:{path}")
    return int(match.group(1)), path


def main() -> None:
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)

    output = Path(sys.argv[1])
    chunks: list[bytes] = []

    for argument in sys.argv[2:]:
        size, path = parse_argument(argument)
        if size not in TYPES_FOR_SIZE:
            raise SystemExit(f"error: unsupported icon size {size} ({path})")

        data = path.read_bytes()
        if not data.startswith(PNG_SIGNATURE):
            raise SystemExit(f"error: {path} is not a PNG")

        for chunk_type in TYPES_FOR_SIZE[size]:
            chunks.append(chunk_type.encode("ascii") + struct.pack(">I", len(data) + 8) + data)

    body = b"".join(chunks)
    output.write_bytes(b"icns" + struct.pack(">I", len(body) + 8) + body)
    print(f"wrote {output} ({output.stat().st_size} bytes)")


if __name__ == "__main__":
    sys.exit(main())
