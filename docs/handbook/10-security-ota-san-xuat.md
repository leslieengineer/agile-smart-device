# 10 — Security, OTA và sản xuất

## Trạng thái

Đây là security design/gate document. Product **chưa production-ready**.

## Trust boundaries

```text
Browser credential boundary
  -> Mosquitto auth/ACL
  -> Gateway validation
  -> Unix socket matter-rpc group
  -> Controller fabric storage
  -> CASE-secured Matter session
  -> Node hardware/persistence
```

## Web/MQTT security

- Anonymous broker access disabled.
- Gateway và WebUI dùng user/ACL khác nhau.
- Browser credential nhập runtime, không bundle.
- QoS1 duplicate phải idempotent.
- Validate envelope/size/unknown fields trước dispatch.
- WSS/TLS/browser authentication production policy vẫn phải được chốt đầy đủ.

## BBB process security

- Controller/Gateway/service users tách biệt.
- Unix socket mode 0660, group `matter-rpc`.
- Fabric storage chỉ Controller đọc.
- systemd hardening và memory limits.
- `/run` ephemeral, `/var/lib` persistent.
- Backup/permission audit trước upgrade.

## Matter credential ownership

| Material | Owner |
|---|---|
| Thread dataset | OTBR/operator commissioning boundary |
| Fabric CA/credentials | Matter Controller storage |
| Operational certificate | Node + Controller fabric |
| Setup passcode/discriminator | Manufacturing/commissioning |
| DAC private key | Per-device protected factory data |
| PAI/CD | Manufacturing/certification chain |

Không đưa bất kỳ material trên vào logs/Git.

## Development credentials

Current image dùng example/test identity/attestation. Chỉ dùng lab bring-up. Production phải có:

- assigned VID/PID;
- unique discriminator/passcode;
- DAC per device;
- PAI và Certification Declaration;
- private key protected;
- QR/manual onboarding payload;
- provisioning audit trail.

## Platform security decisions

Cần chốt và test:

- Secure Boot version/policy.
- Flash Encryption release mode.
- NVS Encryption và key partition.
- JTAG/USB/UART production access.
- Anti-rollback eFuse/version policy.
- Signed build/release artifact custody.
- Recovery/RMA process.

Đốt eFuse là irreversible; chỉ chạy theo manufacturing procedure được review.

## OTA layout

Matter partition table có:

- `otadata`;
- `ota_0` 6 MiB;
- `ota_1` 6 MiB;
- factory data;
- secure certificate;
- coredump.

Đây chỉ là storage readiness. Product chưa có end-to-end signed Matter OTA acceptance.

## OTA acceptance

- signed image accepted;
- bad signature rejected;
- lower version/anti-rollback rejected;
- interrupted download resumes/fails safe;
- power loss trước/sau boot switch;
- new image health confirmation;
- automatic rollback khi boot fail;
- both slots đủ size;
- Controller/provider authorization;
- no credential leak in image/log.

## Manufacturing flow

1. Record board/SoC/flash identity.
2. Program bootloader/partition/application.
3. Provision unique Matter factory data.
4. Configure security eFuses theo approved profile.
5. Verify onboarding code/attestation.
6. Run GPIO/relay/LED/radio test.
7. Store non-secret traceability record.
8. Seal debug access theo product policy.

## Release blockers

- Controller commission/read/subscription API.
- Production VID/PID/attestation.
- Secure boot/encryption/debug decisions.
- Signed OTA/provider/rollback evidence.
- Brownout/relay safety evidence.
- Long-duration Thread/heap/watchdog soak.
- Certification plan.
