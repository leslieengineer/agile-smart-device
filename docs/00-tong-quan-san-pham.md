# Tổng quan sản phẩm

## Phạm vi

Agile Smart Device là hệ thống Matter-over-Thread gồm node ESP32-C6, ứng dụng Android để claim/commission, BBB làm Border Router và permanent Matter Controller, cùng WebUI quản lý thiết bị.

## Trạng thái

| Tính năng | Source | Deployed | HIL |
|---|---|---|---|
| Node On/Off endpoint 1 | Có | Có | Đang debug end-to-end |
| Rhophi claim qua BLE | Có | Có | Đã xác minh từng bước |
| Mobile temporary fabric | Có | Có | Đang debug |
| BBB on-network commission | Có | Có | Chưa đạt acceptance cuối |
| Inventory và On/Off hai chiều | Có | Có | Chưa xác minh sau handoff cuối |
| Restart/remove/recommission | Có một phần | Có một phần | Chưa |

## Actor và ownership

- **ESP32-C6 node** sở hữu relay, button, local persistence và Matter data model.
- **Android app** xác minh ownership, tạo temporary fabric, cấp Thread credentials và mở ECW.
- **BBB OTBR** sở hữu RCP và route Thread/IPv6.
- **Matter.js Controller** sở hữu permanent fabric và subscription.
- **Gateway/BFF** cung cấp MQTT, REST/SSE, auth, provisioning và inventory.
- **WebUI** không giữ Matter, MQTT hay manufacturing secret.

## Phạm vi MVP

1. Claim đúng node bằng physical/auto claim window.
2. Commission node qua BLE vào temporary mobile fabric.
3. Attach node vào Thread network của BBB.
4. BBB commission on-network qua ECW.
5. Mobile xóa temporary fabric.
6. WebUI/mobile hiển thị inventory và điều khiển On/Off hai chiều.
7. Node và BBB restart không mất permanent fabric.

Production attestation, secure boot, flash encryption, signed OTA và formal compliance không thuộc acceptance MVP hiện tại.

## Nguồn sự thật

- `components/product_smart_device/`
- `mobileapp-reference/apps/mobile/`
- `mobileapp-reference/packages/commissioning-plugin/`
- `dashboard-reference/packages/`
- `tests/host/` và `tests/hil/README.md`