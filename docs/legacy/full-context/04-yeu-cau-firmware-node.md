# Yêu cầu Firmware ESP32-C6 Application Node

## Baseline

Firmware node là Matter-over-Thread device, không phải `ot_rcp`.

Cần pin và ghi lại

- ESP-IDF version
- ESP-Matter version/commit
- compiler/tool version
- board/SKU revision
- partition table
- sdkconfig defaults
- Matter device/cluster revisions

Build phải reproducible từ repository sạch.

## Kiến trúc module đề xuất

```text
app_main
  ├─ platform/hardware abstraction
  ├─ actuator or sensor driver
  ├─ device state model
  ├─ local input handling
  ├─ Matter endpoint/cluster server
  ├─ persistence
  ├─ diagnostics/logging
  ├─ commissioning/factory reset
  └─ OTA
```

Không đặt toàn bộ logic trong callback Matter. Callback validate request và chuyển sang state/actuator layer có ownership rõ.

## State model

Một nguồn sự thật phải điều khiển cả hardware output và Matter attributes.

Ví dụ OnOff

1. Matter command yêu cầu `true`.
2. Firmware kiểm tra safety/interlock.
3. Driver đổi output.
4. Chỉ khi output accepted, state model đổi.
5. Matter `OnOff` attribute được update/report.
6. Local button đi qua cùng state transition.

Không chỉ update attribute mà quên hardware, hoặc đổi hardware mà quên attribute.

## Concurrency

Xác định task/context nào sở hữu

- Matter stack callback
- GPIO/button ISR
- motor control
- timer/transition
- NVS writes
- OTA

ISR không gọi API blocking hoặc Matter stack trực tiếp. Chuyển event qua queue/task. State shared cần mutex hoặc single-owner task.

## Persistence

Phân loại state

- volatile runtime state
- state cần phục hồi sau reboot
- Matter fabric/config do stack quản lý
- calibration data
- safety counters/fault history

Không ghi flash mỗi lần slider thay đổi. Dùng debounce/batching để giảm wear.

## Error handling

Firmware phải định nghĩa behavior khi

- Thread detached
- Matter session timeout
- actuator không phản hồi
- sensor out-of-range
- brownout/reboot giữa transition
- NVS corrupt/full
- OTA fail
- watchdog reset

Network mất không được làm local safety function mất hiệu lực.

## Logging và diagnostics

Production log cần

- firmware version/build ID
- reset reason
- commissioning state
- Thread attach/detach reason
- command result không chứa secret
- hardware fault
- heap watermark/task watchdog

Không log setup passcode, operational keys hoặc fabric secret.

## Security

- unique commissioning material theo device
- device attestation theo provisioning strategy
- secure boot/flash encryption theo threat model
- signed OTA và anti-rollback policy
- factory reset xóa fabric credentials đúng chuẩn
- debug interface policy cho production

## OTA

Tài liệu node phải nêu

- image format và partition layout
- signing key ownership
- version comparison
- rollback trigger
- update khi Thread chập chờn
- power-loss tolerance

## Local control

Local button/panel phải hoạt động khi gateway offline. Mọi local state change phải update Matter attribute để controller subscription nhận được.

## Không thuộc firmware node

- MQTT credentials/topics
- WebSocket
- Vue/Pinia state
- BBB serial path
- Spinel radio URL
- `request_id` JSON correlation

Các khái niệm này thuộc Gateway/Web/RCP host boundary.
