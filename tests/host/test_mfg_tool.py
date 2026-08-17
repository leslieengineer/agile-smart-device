from __future__ import annotations

import base64
import json
from pathlib import Path
import sys
import tempfile
import unittest

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives.kdf.hkdf import HKDF

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "mfg"))
from generate_device import generate_record, nvs_csv, validate_output, valid_setup_passcode  # noqa: E402
from export_registry import encrypt_record  # noqa: E402


def decode(value: str) -> bytes:
    return base64.urlsafe_b64decode(value + "=" * (-len(value) % 4))


class ManufacturingToolTests(unittest.TestCase):
    def test_unique_valid_device_material(self):
        records = [generate_record(f"serial-{index:03d}", 1) for index in range(100)]
        self.assertEqual(len({record["claim_id"] for record in records}), 100)
        self.assertEqual(len({record["claim_secret"] for record in records}), 100)
        self.assertEqual(len({record["setup_passcode"] for record in records}), 100)
        for record in records:
            self.assertEqual(len(bytes.fromhex(str(record["claim_id"]))), 16)
            self.assertEqual(len(bytes.fromhex(str(record["claim_secret"]))), 32)
            self.assertTrue(valid_setup_passcode(int(record["setup_passcode"])))
            self.assertIn("rhophi,namespace,,", nvs_csv(record))
            self.assertIn("claim_secret,data,hex2bin", nvs_csv(record))

    def test_registry_matches_backend_aes_gcm_contract(self):
        master = bytes(range(32))
        source = generate_record("device-001", 1)
        encrypted = encrypt_record(master, source)
        claim_id = encrypted["claim_id"]
        key = HKDF(
            algorithm=hashes.SHA256(), length=32,
            salt=claim_id.encode(), info=b"rhophi-registry-v1",
        ).derive(master)
        plaintext = AESGCM(key).decrypt(
            decode(encrypted["nonce"]),
            decode(encrypted["ciphertext"]) + decode(encrypted["authentication_tag"]),
            f"rhophi-registry-v1:{claim_id}".encode(),
        )
        parsed = json.loads(plaintext)
        self.assertEqual(parsed["device_id"], "device-001")
        self.assertEqual(len(decode(parsed["claim_secret"])), 32)

    def test_refuses_output_inside_worktree(self):
        with self.assertRaises(ValueError):
            validate_output(ROOT / "tools" / "mfg" / "out")
        with tempfile.TemporaryDirectory() as directory:
            self.assertEqual(validate_output(Path(directory)), Path(directory).resolve())


if __name__ == "__main__":
    unittest.main()
