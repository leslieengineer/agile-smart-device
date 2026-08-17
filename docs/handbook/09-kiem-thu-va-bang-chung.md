# 09 — Kiểm thử và bằng chứng

## Test pyramid

```text
Host unit tests
  -> architecture/static gates
  -> local/gateway compile profiles
  -> clean Matter build
  -> flash/boot smoke
  -> node HIL
  -> BBB Matter integration
  -> WebUI end-to-end
  -> outage/soak/security/OTA acceptance
```

## Automated host evidence

| Suite | Current evidence |
|---|---|
| Framework | 11/11 pass |
| Parent | 6/6 pass |
| Boundary/dynamic/catalog/CMake | Included trong parent CTest |
| Local ESP-IDF 6 build | Pass |
| Gateway compile-only | Pass |
| Clean Matter build | Pass |
| Matter binary Gateway-string audit | Pass |

Đây là evidence của implementation session, không thay CI result cho commit tương lai.

## Architecture gates

`tools/check_layer_boundaries.py` kiểm tra:

- vendor/RTOS leakage;
- forbidden dependency;
- dynamic/unbounded patterns;
- catalog completeness;
- explicit CMake source;
- public include boundary;
- `main` không truy cập product internals.

Fixture tests bảo vệ checker khỏi regression/false result.

## Matter build evidence

Lưu:

- IDF/ESP-Matter/ConnectedHomeIP SHA;
- product Git SHA và dirty state;
- sdkconfig defaults/config hash;
- partition table;
- firmware binary/ELF SHA;
- size/map;
- compile component inventory;
- binary scan không có Gateway topics.

## Hardware smoke evidence hiện có

**Verified** trên ESP32-C6 revision v0.2:

- flash success;
- boot success;
- endpoint 1 created;
- OpenThread start;
- Matter server listen;
- CHIPoBLE commissionable window;
- reset reason/free heap logs.

Chưa thay thế physical relay/WS2812 HIL.

## Node HIL matrix

Authoritative matrix tại [`tests/hil/README.md`](../../tests/hil/README.md). Tối thiểu gồm:

- erased NVS safe OFF;
- exact short-press counts;
- hold classification;
- ON/OFF power restore;
- reset relay chatter measurement;
- GPIO9 download mode;
- WS2812 lifecycle;
- commissioning/factory reset;
- no watchdog/stack/NVS errors.

## BBB integration matrix

1. Factory-new commission.
2. Descriptor/inventory.
3. Read OnOff.
4. Invoke Off/On/Toggle.
5. Local button subscription event.
6. Controller/Gateway/OTBR/node restart.
7. Thread detach/reattach.
8. Remove/recommission.
9. Invalid endpoint/cluster/command.
10. QoS1 duplicate/idempotency.

Hiện matrix blocked bởi Controller RPC gap.

## Failure injection

- NVS missing/corrupt/commit error.
- Product queue pressure.
- Controller unavailable.
- OTBR restart.
- RCP disconnect/reconnect.
- Broker/Gateway restart.
- Power loss trong delayed persistence.
- Power loss trong OTA.
- Invalid signature/version rollback.

## Bằng chứng HIL cần lưu

| Field | Ví dụ loại dữ liệu |
|---|---|
| Build identity | firmware SHA, ELF SHA, toolchain SHAs |
| Hardware identity | board revision, chip revision, serial/MAC sanitized |
| Network identity | fabric label, operational Node ID; không lưu key |
| Data model | Descriptor/endpoint/cluster dump |
| Logs | node, Controller, Gateway, OTBR timestamped |
| Physical | relay/LED observation, scope capture |
| Result | pass/fail, expected/actual, issue link |

## Không được claim

- Workflow file tồn tại ≠ CI pass.
- Build pass ≠ flash pass.
- Boot pass ≠ commissioning pass.
- Commission pass ≠ subscription/recovery pass.
- Test credential ≠ production security.
- OTA partition ≠ signed OTA readiness.
