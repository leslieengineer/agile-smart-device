# Hồ sơ Node, Endpoint và Cluster

## Mô hình mock hiện tại

Mock dùng một node tổng hợp để WebUI phát triển nhanh.

| Node | Endpoint | Cluster |
|---|---:|---|
| `0x0000000000000001` | 1 | OnOff `0x0006`, LevelControl `0x0008` |
| `0x0000000000000001` | 2 | WindowCovering `0x0102` |
| `0x0000000000000001` | 3 | VendorCooktop `0xfc01` |

Production không nhất thiết gom mọi thiết bị vào một node. Mỗi SKU ESP32-C6 thường là một Matter node riêng, có endpoint 0 root và một hoặc nhiều application endpoint.

## Endpoint 0

Matter node cần endpoint 0 cho root/node management clusters theo ESP-Matter composition. Nhóm firmware không dùng endpoint 0 cho công tắc/rèm/bếp application behavior.

## Profile A — Smart switch/light

### Clusters hiện được Dashboard dùng

- OnOff `0x0006`
- LevelControl `0x0008` nếu hardware hỗ trợ dimming

### Commands

| Cluster | Command | ID | Payload |
|---|---|---:|---|
| OnOff | Off | `0x00` | `{}` |
| OnOff | On | `0x01` | `{}` |
| OnOff | Toggle | `0x02` | `{}` |
| LevelControl | MoveToLevel | `0x00` | `level 0..254`, optional `transitionTime 0..65534` |
| LevelControl | Move | `0x01` | `moveMode 0..1`, optional `rate 0..254` |
| LevelControl | Step | `0x02` | `stepMode 0..1`, `stepSize 0..254`, optional `transitionTime` |
| LevelControl | Stop | `0x03` | `{}` |
| LevelControl | MoveToLevelWithOnOff | `0x04` | giống MoveToLevel |

### Attributes WebUI hiện đọc

- `OnOff`
- `CurrentLevel`

Firmware phải cập nhật attribute khi local button hoặc dimmer thay đổi output.

## Profile B — Window covering

Cluster WindowCovering `0x0102`.

| Command | ID | Payload |
|---|---:|---|
| UpOrOpen | `0x00` | `{}` |
| DownOrClose | `0x01` | `{}` |
| StopMotion | `0x02` | `{}` |
| GoToLiftPercentage | `0x05` | `liftPercent100ths 0..10000` |
| GoToTiltPercentage | `0x08` | `tiltPercent100ths 0..10000` |

WebUI mock đọc `CurrentPositionLiftPercent100ths`.

Firmware production cần thêm safety và calibration behavior

- travel limits
- obstruction detection
- motor timeout
- position unknown/un-calibrated state
- power-loss recovery
- stop có ưu tiên cao

Tên payload/attribute trong gateway phải được đối chiếu chính xác với ESP-Matter generated API và Matter data types trước freeze.

## Profile C — Cooktop

Hiện là prototype vendor-specific

- test vendor ID `0xfff1`
- cluster ID `0xfc01`

| Command | ID | Payload |
|---|---:|---|
| SetZonePower | `0x00` | `vendor_id`, `zone 0..3`, `powerLevel 0..9` |
| SetBoost | `0x01` | `vendor_id`, `zone 0..3`, `enabled` |
| LockPanel | `0x02` | `vendor_id`, `locked` |
| StopAll | `0x03` | `vendor_id` |

Attributes mock

- `PanelLocked`
- `ZonePower` array bốn phần tử
- `Boost` array bốn phần tử

## Cảnh báo safety và compliance

Cooktop là thiết bị công suất cao. Contract demo không đủ để ship. Cần hazard analysis, fail-safe state, watchdog, thermal sensor, independent power cutoff và quy tắc local control ưu tiên. Không dùng test vendor ID trong production.

## Quy tắc profile firmware

Mỗi SKU phải giao một bảng cố định gồm

- Matter device type và revision
- endpoint list
- server/client clusters
- mandatory/optional attributes
- accepted commands
- persistent attributes
- local input behavior
- boot/factory reset behavior
- error/safety states
