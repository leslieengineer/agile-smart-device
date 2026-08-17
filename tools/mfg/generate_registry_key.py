#!/usr/bin/env python3
"""Generate a private 32-byte provisioning registry key without printing it."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import secrets

from generate_device import validate_output


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()
    output = validate_output(args.out)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(secrets.token_bytes(32))
    try:
        os.chmod(output, 0o600)
    except OSError:
        pass
    print(f"Generated private registry key at {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
