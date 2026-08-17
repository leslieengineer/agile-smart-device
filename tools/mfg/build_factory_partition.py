#!/usr/bin/env python3
"""Invoke ESP-IDF's NVS generator for a private factory CSV."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys

from generate_device import validate_output


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--size", default="0x6000")
    args = parser.parse_args()
    output = validate_output(args.out)
    idf_path = Path(os.environ.get("IDF_PATH", ""))
    generator = idf_path / "components" / "nvs_flash" / "nvs_partition_generator" / "nvs_partition_gen.py"
    if not generator.is_file():
        raise SystemExit("IDF_PATH does not contain nvs_partition_gen.py")
    output.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([sys.executable, str(generator), "generate", str(args.csv.resolve()), str(output), args.size], check=True)
    try:
        os.chmod(output, 0o600)
    except OSError:
        pass
    print(f"Generated private factory partition at {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
