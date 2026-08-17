#!/usr/bin/env python3
"""Generate unique Rhophi claim and Matter onboarding material without logging secrets."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import secrets

INVALID_PASSCODES = {
    0, 11111111, 22222222, 33333333, 44444444,
    55555555, 66666666, 77777777, 88888888,
    12345678, 87654321,
}
REPO_ROOT = Path(__file__).resolve().parents[2]


def valid_setup_passcode(value: int) -> bool:
    return 1 <= value <= 99_999_998 and value not in INVALID_PASSCODES


def derive_claim_id(chip_mac: str) -> str:
    normalized = "".join(chip_mac.split(":")).replace("-", "").lower()
    if len(normalized) != 12:
        raise ValueError("chip MAC must contain exactly 6 bytes")
    try:
        mac = bytes.fromhex(normalized)
    except ValueError as error:
        raise ValueError("chip MAC must be hexadecimal") from error
    return hashlib.sha256(b"rhophi-claim-id-v1:" + mac).digest()[:16].hex()


def generate_record(
    serial: str,
    chip_mac: str,
    product_id: int,
    setup_passcode: int | None = None,
    discriminator: int | None = None,
) -> dict[str, object]:
    if not serial or len(serial) > 64:
        raise ValueError("serial must contain 1-64 characters")
    if not 1 <= product_id <= 0xFFFF:
        raise ValueError("product ID must be 1-65535")
    passcode = setup_passcode or 0
    if setup_passcode is None:
        while not valid_setup_passcode(passcode):
            passcode = secrets.randbelow(99_999_998) + 1
    elif not valid_setup_passcode(passcode):
        raise ValueError("setup passcode is invalid")
    if discriminator is not None and not 0 <= discriminator <= 4095:
        raise ValueError("discriminator must be 0-4095")
    selected_discriminator = discriminator if discriminator is not None else secrets.randbelow(4096)
    return {
        "version": 1,
        "serial": serial,
        "device_id": serial,
        "chip_mac": chip_mac,
        "product_id": product_id,
        "claim_id": derive_claim_id(chip_mac),
        "claim_secret": secrets.token_hex(32),
        "setup_passcode": passcode,
        "discriminator": selected_discriminator,
    }


def validate_output(path: Path) -> Path:
    resolved = path.expanduser().resolve()
    if resolved == REPO_ROOT or REPO_ROOT in resolved.parents:
        raise ValueError("manufacturing output must be outside the Git worktree")
    return resolved


def write_private(path: Path, content: str) -> None:
    path.write_text(content, encoding="utf-8")
    try:
        os.chmod(path, 0o600)
    except OSError:
        pass


def nvs_csv(record: dict[str, object]) -> str:
    return "\n".join([
        "key,type,encoding,value",
        "rhophi,namespace,,",
        f"product_id,data,u16,{record['product_id']}",
        f"claim_id,data,hex2bin,{record['claim_id']}",
        f"claim_secret,data,hex2bin,{record['claim_secret']}",
        "",
    ])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--serial", required=True)
    parser.add_argument("--chip-mac", required=True, help="6-byte base MAC/eFuse identity")
    parser.add_argument("--product-id", required=True, type=lambda value: int(value, 0))
    parser.add_argument("--setup-passcode", type=int)
    parser.add_argument("--discriminator", type=int)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()

    output = validate_output(args.out)
    output.mkdir(parents=True, exist_ok=True)
    record = generate_record(
        args.serial,
        args.chip_mac,
        args.product_id,
        args.setup_passcode,
        args.discriminator,
    )
    stem = args.serial.replace("/", "_").replace("\\", "_")
    write_private(output / f"{stem}.device.json", json.dumps(record, indent=2) + "\n")
    write_private(output / f"{stem}.fctry.csv", nvs_csv(record))
    print(f"Generated private manufacturing files for {args.serial} in {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
