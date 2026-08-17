# Full Context — Smart Home Gateway và ESP32-C6 Nodes

Tài liệu này là gói bàn giao kỹ thuật giữa nhóm Gateway/Web và nhóm firmware ESP32-C6 application node. Nó tóm lược những gì đã làm, đang làm, sẽ làm, cùng các contract cần thống nhất trước khi tích hợp phần cứng thật.

Đây là shared snapshot trong firmware repository. Source/as-built canonical của WebUI/BBB nằm tại `C:\Users\lesli\WS\agile-dashboard`; các path `packages/`, `apps/` và `deploy/` phía dưới được hiểu là relative tới sibling repository đó. Người mới nên bắt đầu tại [`docs/README.md`](../README.md).

## Đọc nhanh theo vai trò

### Quản lý kỹ thuật

1. [Trạng thái và quyết định](00-trang-thai-va-quyet-dinh.md)
2. [Kiến trúc hệ thống](01-kien-truc-he-thong.md)
3. [Roadmap và vấn đề còn mở](07-roadmap-va-open-issues.md)

### Nhóm firmware ESP32-C6 node

1. [Hồ sơ node, endpoint và cluster](02-node-endpoint-cluster.md)
2. [Yêu cầu firmware node](04-yeu-cau-firmware-node.md)
3. [Thread, commissioning và lifecycle](05-thread-commissioning.md)
4. [Tiêu chí kiểm thử tích hợp](06-kiem-thu-tich-hop.md)
5. [Checklist bàn giao](08-checklist-ban-giao.md)

### Nhóm Gateway/Web

1. [Contract MQTT và gateway translation](03-contract-mqtt-gateway.md)
2. [Kiến trúc hệ thống](01-kien-truc-he-thong.md)
3. [Tiêu chí kiểm thử tích hợp](06-kiem-thu-tich-hop.md)

## Nguyên tắc bắt buộc

- ESP32-C6 RCP và ESP32-C6 application node là hai vai trò khác nhau
- RCP chạy `ot_rcp`, không chứa endpoint Smart Home
- application node chạy Matter device firmware, không nhận JSON MQTT trực tiếp
- MQTT chỉ tồn tại giữa WebUI, broker và BBB gateway
- BBB Matter Controller chuyển MQTT contract thành Matter Interaction Model
- ưu tiên standard Matter cluster; vendor cluster chỉ dùng khi không có semantic chuẩn phù hợp
- trạng thái hiển thị phải đến từ Matter attribute report/read response, không chỉ từ giả định UI

## Nguồn sự thật trong repository

| Nội dung | File |
|---|---|
| Message envelope | `packages/contracts/src/envelope.ts` |
| Cluster/command IDs hiện tại | `packages/contracts/src/ids.ts` |
| Payload validation | `packages/contracts/src/clusters/*` |
| Translation registry | `packages/gateway/src/clusters/*` |
| Controller abstraction | `packages/gateway/src/controller/MatterController.ts` |
| Mô phỏng hiện tại | `packages/gateway/src/controller/MockMatterController.ts` |
| UI widgets | `apps/webui/src/components/*` |
| BBB deployment | `deploy/README.md` |

## Cảnh báo về tính chuẩn

Mock model hiện tại phục vụ phát triển Web. Nó không phải specification cuối cùng cho firmware. Trước production, nhóm firmware và gateway phải đối chiếu Matter specification/ESP-Matter SDK về device type, endpoint composition, attribute ID, command payload và data type.
