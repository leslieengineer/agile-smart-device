# Trạng thái và quyết định

Cập nhật ngày 2026-08-16.

## Đã hoàn thành

### WebUI

- Vue 3 + Vite + TypeScript
- TailwindCSS và responsive Dashboard
- Pinia stores cho connection, devices và activity
- mqtt.js qua WebSocket
- widget OnOff, LevelControl, WindowCovering và Cooktop
- correlation bằng `request_id`
- validation response/event bằng Zod

### Gateway mock

- Node.js MQTT bridge
- topic `home/control/tx`, `home/control/rx`, `home/control/status`
- envelope validation và giới hạn 8 KiB
- cluster/command registry mở rộng theo module
- controller abstraction
- MockMatterController phát response và attribute event
- structured JSON logging
- timeout và typed error model

### Chất lượng

- TypeScript strict
- 10 test qua 5 test files
- integration test dùng authenticated in-process MQTT broker
- production build WebUI/gateway/contracts thành công

### BBB và Thread RCP

- BBB Debian 11 `armv7l`
- SSH key-based access hoạt động
- Node.js 20.20.2 được cài user-local
- ESP32-C6 RCP đã build lại với `CONFIG_OPENTHREAD_RCP_USB_SERIAL_JTAG=y`
- USB RCP ở `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_98:A3:16:AA:96:9C-if00`
- Pyspinel đọc được version RCP ở 460800 baud
- source OTBR được clone và bootstrap trên BBB
- `otbr-agent.service` active, `wpan0` up và OTBR role là leader
- Matter Controller matter.js 0.17.9, Matter Gateway, WebUI và Mosquitto chạy bằng systemd
- Controller/fabric storage đặt tại `/var/lib/matter-controller`

### ESP32-C6 application node

- Chốt On/Off Plug-in Unit trên endpoint ứng dụng, OnOff server cluster `0x0006`
- Pin ESP-IDF v5.5.5 và ESP-Matter release/v1.6 bằng commit SHA
- Upstream Thread reference build, flash và mở commissioning trên ESP32-C6 revision v0.2
- Product `matter_node` build, flash và boot; OpenThread/Matter server khởi động
- Matter write đi qua bounded product queue; local change được schedule về Matter attribute
- Button short/commissioning/factory-reset gestures và WS2812 GPIO8 lifecycle indicator đã có implementation

## Đang thực hiện

- bổ sung `commission`, `removeNode`, explicit `read` và subscription/event API cho BBB Matter Controller
- commission application node vào đúng fabric của BBB
- validate WebUI → MQTT → Gateway → Matter → Thread → relay và local attribute report chiều ngược lại
- HIL long-press thresholds, power restore, Thread outage và restart recovery
- production attestation, security và OTA evidence

Các mục này chưa hoàn thành cho tới khi có output/log/HIL verification tương ứng.

## Chưa thực hiện

- BBB Controller commissioning/remove/read/subscription API hoàn chỉnh
- commissioning product ESP32-C6 node vào fabric BBB
- Gateway subscription forwarding cho unsolicited local OnOff report
- persistent node inventory và subscription recovery đã kiểm chứng qua reboot
- WSS/TLS cho browser MQTT
- production vendor ID và vendor cluster specification
- OTA, diagnostics, recovery và manufacturing provisioning cho nodes

## Quyết định đã chốt

1. BBB là Linux Gateway và OpenThread Border Router host.
2. ESP32-C6 chuyên dụng làm RCP qua USB Spinel.
3. Application nodes là Matter-over-Thread devices riêng.
4. Node firmware không biết MQTT topic hoặc WebUI JSON.
5. Gateway không mở RCP serial; `otbr-agent` sở hữu độc quyền.
6. Contract Web dùng Node ID 64-bit dạng hex string.
7. Numeric Matter IDs là canonical; tên symbolic chỉ dùng tại biên Web.
8. Bản đầu chạy mock để phát triển frontend/backend độc lập phần cứng.

## Giả định cần xác nhận với nhóm node

- ESP-IDF v5.5.5 và ESP-Matter release/v1.6 đã pin; upgrade cần chạy lại gates
- board variant và antenna vẫn cần hardware evidence
- relay GPIO10, button GPIO9, relay LED GPIO2 và WS2812 GPIO8 đã chốt cho SKU đầu
- nguồn điện và brownout behavior chưa có HIL evidence
- SKU đầu dùng endpoint On/Off Plug-in Unit; SKU sau vẫn phải chốt riêng
- commissioning dùng BLE + Thread credentials qua Matter flow; production material chưa provision
- factory reset dùng released hold ít nhất 10 giây; threshold cần HIL validation
- OTA transport và signing
- fail-safe behavior của bếp/rèm/công tắc
