# Roadmap và Open Issues

## Phase 0 — Web/Gateway mock

Trạng thái hoàn thành.

- contract package
- Vue Dashboard
- Pinia realtime state
- MQTT bridge
- command registry
- mock controller
- tests và build

## Phase 1 — Thread infrastructure

Trạng thái **đã kiểm chứng trên BBB**.

- OTBR/RCP installed, `otbr-agent` active
- active Thread dataset tồn tại nhưng không lưu key trong repository
- `wpan0`/IPv6 route hoạt động, OTBR role leader
- Mosquitto production và systemd services active
- RCP chỉ do `otbr-agent` sở hữu

Exit criteria

- RCP owner chỉ là otbr-agent
- Thread role hợp lệ
- reboot BBB tự phục hồi services
- health check và logs rõ

## Phase 2 — Matter Controller

Trạng thái **đang thực hiện**.

Đã có:

- matter.js 0.17.9 Controller service
- persistent storage `/var/lib/matter-controller`
- Unix RPC socket và systemd hardening
- `health`, `listNodes`, `invoke`

Còn thiếu:

- commissioning và remove/decommission API
- explicit read và subscription/event stream
- node inventory/subscription recovery verification
- status/error mapping và tests với node thật

Không dùng CLI spawn per-command làm production architecture nếu không có session/subscription lifecycle rõ.

## Phase 3 — Smart switch node MVP

Trạng thái **implemented/build/boot verified, HIL integration pending**.

Đã có:

- ESP32-C6 Matter-over-Thread firmware
- On/Off Plug-in Unit endpoint và local button
- bounded local/remote state owner và attribute reporting code
- commissioning/factory-reset gestures
- WS2812 lifecycle indication
- dual-slot OTA partition skeleton

Còn thiếu:

- BBB commissioning/read/invoke/subscription
- physical relay/long-press/power/Thread HIL
- signed OTA/security production gates

LevelControl chỉ thêm nếu hardware SKU hỗ trợ. Đây là node đầu tiên nên tích hợp vì risk thấp hơn motor và cooktop.

## Phase 4 — Window covering

- motor driver và calibration
- limit/obstruction safety
- position model
- standard cluster conformance
- long-running command/report behavior

## Phase 5 — Cooktop prototype

Chỉ bắt đầu sau hazard analysis và production vendor identity.

- đánh giá standard clusters có thể dùng
- vendor cluster specification versioned
- independent safety controller/interlock
- thermal/current sensing
- compliance plan

## Open issues cần owner và deadline nội bộ

| ID | Vấn đề | Owner đề xuất |
|---|---|---|
| OI-01 | Chọn Matter Controller SDK/service trên BBB armv7 | Gateway |
| OI-02 | Node inventory/fabric storage format | Gateway |
| OI-03 | ESP-IDF/ESP-Matter pinned version | Firmware |
| OI-04 | Production VID/PID và attestation | Product/Security |
| OI-05 | Device type/endpoint map mỗi SKU | Firmware + Gateway |
| OI-06 | Attribute/command naming map SDK ↔ JSON | Gateway |
| OI-07 | OTA signing/provisioning | Firmware/Security |
| OI-08 | WSS certificate và browser auth | Platform |
| OI-09 | Cooktop safety/compliance scope | Product/Safety |
| OI-10 | Node offline/timeout UX | Web/Gateway |
| OI-11 | Duplicate QoS1 command policy | Gateway/Firmware |
| OI-12 | Production Node runtime path/systemd fix | Platform |

## Contract freeze gate

Không freeze firmware interface chỉ từ MockMatterController. Freeze sau khi

1. Matter device types được chọn
2. ESP-Matter endpoint composition compile được
3. Controller discover/read thử thành công
4. command payload map tới SDK types
5. node team và gateway team review
6. test vector được lưu

## Definition of done toàn hệ thống

- node thật commission và điều khiển qua Matter
- local state change cập nhật Web realtime
- reboot BBB/node tự phục hồi
- security credentials không lộ
- services production chạy systemd
- observability đủ chẩn đoán từng boundary
- tests tự động và HIL evidence đạt
- documentation/runbook cập nhật theo release
