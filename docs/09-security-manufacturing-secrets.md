# Security, manufacturing và secrets

## Trạng thái

| Hạng mục | Source | Deployed | HIL/Release |
|---|---|---|---|
| Per-device claim material | Có | Lab | Manufacturing test |
| Encrypted BBB registry | Có | Có | Unit test |
| Grant encryption | Có | Có | Claim flow xác nhận |
| Production attestation | Chưa hoàn chỉnh | Không | Blocker |
| Secure boot/flash encryption | Chưa bật | Không | Blocker |
| Signed OTA | Policy một phần | Không | Blocker |

## Phân loại secret

| Secret | Nơi sống | Không bao giờ vào |
|---|---|---|
| Claim secret | `fctry` + encrypted registry | Git, MQTT, log |
| Registry master key | protected BBB file | WebUI, evidence |
| Thread dataset | OTBR/provider/grant plaintext tạm | Git, chat, log |
| Mobile fabric key | Android CHIP storage | BFF, MQTT |
| BBB fabric key | Matter Controller storage | client app |
| DAC/PAI private key | manufacturing secure store | firmware source |
| Admin/cloud token | protected env/file | screenshot/docs |

## Manufacturing pipeline

1. Đọc base MAC của chip.
2. `generate_device.py` tạo claim ID, secret, product/setup data ngoài worktree.
3. `build_factory_partition.py` tạo NVS image `fctry`.
4. `generate_registry_key.py` tạo master key.
5. `export_registry.py` tạo AES-256-GCM registry envelope.
6. Flash factory partition và cài registry/key với permission chặt.

Claim ID được derive từ MAC theo context versioned; claim secret là ngẫu nhiên 32 byte.

## Grant

BFF dùng X25519 ephemeral DH, HKDF-SHA256 và AES-256-GCM. Transaction ID bind key/AAD; grant có expiry. Plaintext chứa setup data và Thread dataset, được zeroize sau dùng.

## Lab và production

Passcode/discriminator test CHIP chỉ dùng lab. Debug app có thể cho phép development attestation. Production phải có unique onboarding material, DAC/PAI/CD, PAA trust policy, secure boot, flash encryption, encrypted factory storage và signed OTA.

## Redaction

Không chạy hoặc paste `ot-ctl dataset active -x`. Evidence chỉ ghi dataset tồn tại/độ dài hợp lệ. Loggers phải redact password, bearer, cookie, grant, challenge, proof, passcode và dataset.

## Threat boundaries

Physical claim gesture/claim window hạn chế takeover. Replay cache bảo vệ challenge; BFF rate-limit invalid proof. Node không tự lock valid proof vì không biết backend verdict.

## Nguồn sự thật

- `tools/mfg/`
- `components/product_smart_device/src/matter/RhophiClaimPlatform.cpp`
- `dashboard-reference/packages/provisioning/src/`
- `partitions_matter.csv`