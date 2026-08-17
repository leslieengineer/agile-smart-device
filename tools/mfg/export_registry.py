#!/usr/bin/env python3
"""Encrypt per-device records for the BBB provisioning registry."""

from __future__ import annotations

import argparse
import base64
import json
import os
from pathlib import Path
import secrets

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives.kdf.hkdf import HKDF

from generate_device import validate_output, valid_setup_passcode

INFO = b"rhophi-registry-v1"


def b64url(value: bytes) -> str:
    return base64.urlsafe_b64encode(value).rstrip(b"=").decode("ascii")


def load_master_key(path: Path) -> bytes:
    value = path.read_bytes()
    stripped = value.strip()
    try:
        key = bytes.fromhex(stripped.decode("ascii")) if len(stripped) == 64 else value
    except (UnicodeDecodeError, ValueError):
        key = value
    if len(key) != 32:
        raise ValueError("registry key must be exactly 32 bytes or 64 hexadecimal characters")
    return key


def derive_key(master: bytes, claim_id: str) -> bytes:
    return HKDF(algorithm=hashes.SHA256(), length=32, salt=claim_id.encode(), info=INFO).derive(master)


def encrypt_record(master: bytes, source: dict[str, object]) -> dict[str, str]:
    claim_hex = str(source["claim_id"])
    secret_hex = str(source["claim_secret"])
    claim = bytes.fromhex(claim_hex)
    secret = bytes.fromhex(secret_hex)
    if len(claim) != 16 or len(secret) != 32:
        raise ValueError("claim ID/secret lengths are invalid")
    product_id = int(source["product_id"])
    discriminator = int(source["discriminator"])
    passcode = int(source["setup_passcode"])
    if not 1 <= product_id <= 0xFFFF or not 0 <= discriminator <= 4095 or not valid_setup_passcode(passcode):
        raise ValueError("device record contains invalid Matter values")
    claim_id = b64url(claim)
    plaintext = json.dumps({
        "product_id": product_id,
        "claim_secret": b64url(secret),
        "setup_passcode": passcode,
        "discriminator": discriminator,
        "device_id": str(source["device_id"]),
    }, separators=(",", ":")).encode()
    nonce = secrets.token_bytes(12)
    key = derive_key(master, claim_id)
    aad = f"rhophi-registry-v1:{claim_id}".encode()
    encrypted = AESGCM(key).encrypt(nonce, plaintext, aad)
    return {
        "claim_id": claim_id,
        "nonce": b64url(nonce),
        "ciphertext": b64url(encrypted[:-16]),
        "authentication_tag": b64url(encrypted[-16:]),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--key", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("records", nargs="+", type=Path)
    args = parser.parse_args()
    output = validate_output(args.out)
    master = load_master_key(args.key)
    records = [encrypt_record(master, json.loads(path.read_text(encoding="utf-8"))) for path in args.records]
    claim_ids = [record["claim_id"] for record in records]
    if len(set(claim_ids)) != len(claim_ids):
        raise ValueError("duplicate claim ID in registry export")
    envelope = {"version": 1, "algorithm": "AES-256-GCM", "records": records}
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(envelope, separators=(",", ":")) + "\n", encoding="utf-8")
    try:
        os.chmod(output, 0o600)
    except OSError:
        pass
    print(f"Encrypted {len(records)} device records into {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
