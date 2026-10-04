# Rhophi claim protocol v1

> **LEGACY / NON-AUTHORITATIVE.** Dùng [Rhophi claim GATT authoritative](../03-rhophi-claim-gatt.md).

This contract is shared by the ESP32-C6 firmware, the native mobile commissioner, and the BBB provisioning service. Matter setup credentials and the Thread Operational Dataset are never carried by this GATT service.

## Discovery and physical presence

The device accepts claim operations only during the 900-second physical-presence window opened by the five-second local-button gesture. Mobile scans Matter BLE advertisements, connects, discovers the Rhophi service, and reads `DeviceIdentity`. A future scan-response hint may be added only through a supported ConnectedHomeIP API; it cannot replace reading the authoritative identity characteristic.

## GATT service

| Item | UUID | Access | Encoding |
|---|---|---|---|
| Service | `9a7d5210-8e21-4f41-a131-52484f504849` | Primary | — |
| DeviceIdentity | `9a7d5211-8e21-4f41-a131-52484f504849` | Read | 36 bytes |
| ClaimChallenge | `9a7d5212-8e21-4f41-a131-52484f504849` | Write | 32 bytes |
| ClaimResponse | `9a7d5213-8e21-4f41-a131-52484f504849` | Read/notify | 32 bytes |
| CommissioningState | `9a7d5214-8e21-4f41-a131-52484f504849` | Read/notify | 1 byte |
| Identify | `9a7d5215-8e21-4f41-a131-52484f504849` | Write | Empty payload |
| Cancel | `9a7d5216-8e21-4f41-a131-52484f504849` | Write | Empty payload |

UUIDs above use canonical text representation. NimBLE's `BLE_UUID128_INIT` source bytes are little-endian storage bytes and must not be copied directly as text.

## DeviceIdentity layout

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | Protocol version, currently `1` |
| 1 | 2 | Product ID, unsigned little-endian |
| 3 | 16 | Claim ID |
| 19 | 16 | Device nonce |
| 35 | 1 | Flags |

Flag bits are `0x01` commissionable, `0x02` factory-new, `0x04` claimed, and `0x08` locked-out. Reserved bits must be zero. When the physical window is closed, commissionable is clear and the nonce must not be accepted for proof generation.

## Claim proof

1. Mobile creates an authenticated BBB session with claim ID, product ID, and its ephemeral X25519 SPKI public key.
2. BBB returns a random 32-byte challenge and transaction expiry.
3. Mobile reads `DeviceIdentity` and retains the 16-byte nonce.
4. Mobile writes the challenge to `ClaimChallenge`.
5. Firmware computes:

```text
HMAC-SHA256(device_claim_secret,
            device_nonce[16] || server_challenge[32] || claim_id[16])
```

6. Firmware returns the 32-byte proof and rotates its nonce.
7. Mobile submits the retained pre-challenge nonce and proof to BBB using unpadded base64url.
8. BBB verifies in constant time, consumes the transaction challenge/nonce, and returns an encrypted commissioning grant.

Firmware issuing a proof does not mean the claim was verified. Only BBB owns verification. Firmware persists `claimed` only after successful Matter commissioning.

## Session and abuse controls

- One BLE connection owns the active claim exchange.
- Other connections may read non-secret discovery state but cannot write challenge/cancel or read another connection's response.
- A disconnect releases the connection binding but does not extend the physical window.
- Challenge replay is rejected using a bounded cache.
- Failed-attempt lockout survives reboot and cannot be reset by reopening the physical window.
- Cancel, expiry, disconnect, factory reset, and Matter-window closure clear transient challenge/proof buffers.
- No BLE bonding is required; physical presence, HMAC authentication, per-connection binding, TTL, replay protection, and rate limiting form the pre-PASE protection boundary.

## REST binary encoding

Claim ID, nonce, challenge, proof, public keys, ciphertext, authentication tag, and Thread Dataset are encoded as RFC 4648 base64url without padding. APIs reject wrong decoded lengths before cryptographic processing.

## Shared known-answer vector

```text
secret    = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce     = 202122232425262728292a2b2c2d2e2f
challenge = 303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f
claim_id  = 505152535455565758595a5b5c5d5e5f
proof     = 66cd0ee63055effd24c52b90779f9a43e1e1532844604980435267fcb4517027
```
