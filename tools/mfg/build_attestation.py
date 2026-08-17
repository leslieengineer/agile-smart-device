#!/usr/bin/env python3
"""Fail-closed wrapper around the approved Espressif Matter manufacturing tool."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess

from generate_device import validate_output


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--device-record", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("extra", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    output = validate_output(args.out)
    tool_value = os.environ.get("ESP_MATTER_MFG_TOOL")
    if not tool_value:
        raise SystemExit("ESP_MATTER_MFG_TOOL must point to the reviewed Espressif esp-matter-tools mfg_tool")
    tool = Path(tool_value)
    if not tool.is_file():
        raise SystemExit("ESP_MATTER_MFG_TOOL is not a file")
    output.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(tool), "--device-record", str(args.device_record.resolve()), "--out", str(output), *args.extra], check=True)
    print(f"Generated attestation artifacts in {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
