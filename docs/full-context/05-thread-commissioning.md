# Thread, Commissioning và Node Lifecycle

## Điều kiện trước commissioning

- OTBR service healthy
- ESP32-C6 RCP trả lời và chỉ OTBR mở device
- `wpan0` tồn tại
- Thread Operational Dataset active
- IPv6 forwarding/routing đúng
- Matter Controller có persistent storage
- node firmware ở trạng thái factory-new

## Thread dataset

Dataset chứa thông tin mạng Thread như channel, PAN ID, network key và mesh-local prefix. Đây là secret vận hành, không commit vào repository hoặc log công khai.

Application node không hard-code dataset production. Trong Matter commissioning, commissioner cung cấp Thread Network Credentials cho node theo flow chuẩn.

## Matter commissioning flow

```text
factory-new node
  → commissioning window
  → discovery/onboarding
  → PASE session bằng setup code
  → device attestation
  → provision Thread credentials
  → node attach Thread
  → provision Matter fabric operational credentials
  → CASE operational session
  → endpoint discovery/subscription
```

Chi tiết API phụ thuộc ESP-Matter và Controller SDK được chọn.

## Node ID

Operational Node ID do fabric/controller cấp khi commission. Firmware không dùng mock ID `0x1` làm hằng production. Cùng thiết bị có thể có identity theo fabric; inventory gateway phải lưu fabric context và node ID.

## Lifecycle states

### Factory new

- chưa thuộc fabric
- commissioning window mở theo policy
- output ở safe/default state

### Commissioning

- PASE/attestation
- nhận network/fabric credentials
- có timeout/fail-safe
- thất bại phải rollback partial provisioning

### Operational

- Thread attached
- CASE session theo nhu cầu
- attributes/subscriptions hoạt động
- local control độc lập gateway

### Offline/recovering

- không xóa fabric chỉ vì mất network
- retry có backoff
- giữ local safety behavior
- báo diagnostics khi kết nối lại

### Factory reset

- xóa fabrics và network credentials theo SDK
- giữ hoặc xóa calibration theo product policy rõ
- không để actuator chuyển trạng thái nguy hiểm
- có physical gesture chống reset nhầm

## Commissioning artifacts nhóm node phải giao

- QR code và manual pairing code cho development units
- discriminator/passcode generation policy
- Device Attestation strategy
- vendor/product IDs development và production
- device type và endpoint map
- factory reset procedure
- commissioning window reopen procedure
- serial/log method không lộ secret

## Gateway responsibilities

- commission/remove nodes
- lưu fabric credentials an toàn
- lưu inventory và human-readable name
- establish CASE session
- subscribe required attributes
- resync state sau restart
- không publish Thread dataset lên MQTT

## RCP/OTBR responsibilities

RCP không commission application node. Nó chỉ cung cấp radio. OTBR vận chuyển IPv6/Thread và quản lý Thread host role; Matter Controller thực hiện Matter commissioning phía application.

## Bring-up order đề xuất

1. xác nhận RCP version
2. xác nhận otbr-agent healthy
3. tạo/commit/start Thread dataset
4. xác nhận role leader/router
5. chạy Matter Controller service
6. flash factory-new application node
7. commission node
8. read Descriptor/Basic Information
9. subscribe application attributes
10. mới nối Gateway MQTT adapter
