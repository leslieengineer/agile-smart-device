# Trung tâm tài liệu kỹ thuật Agile Smart Device

Đây là điểm bắt đầu dành cho thành viên mới tham gia dự án. Bộ tài liệu mô tả toàn bộ sản phẩm từ WebUI, BBB Gateway, Matter Controller, OTBR đến ESP32-C6 application node.

## Trạng thái tài liệu

Cập nhật theo source và kết quả kiểm chứng ngày 2026-08-16.

| Nhãn | Ý nghĩa |
|---|---|
| **Verified** | Đã có command output, test, build hoặc hardware log. |
| **Implemented, chưa HIL** | Code đã tồn tại và compile nhưng chưa qua acceptance trên toàn hệ thống. |
| **Planned** | Có thiết kế/roadmap nhưng chưa có implementation hoàn chỉnh. |
| **Production blocker** | Bắt buộc hoàn thành trước khi phát hành sản phẩm. |
| **Legacy/reference-only** | Chỉ dùng đối chiếu, không thuộc kiến trúc active. |

## Lộ trình đọc

### Đọc nhanh trong 30 phút

1. [Bắt đầu](handbook/00-bat-dau.md)
2. [Sản phẩm và use case](handbook/01-san-pham-va-use-case.md)
3. [Kiến trúc toàn hệ thống](handbook/02-kien-truc-toan-he-thong.md)
4. [Bản đồ kiến trúc end-to-end chi tiết](architecture/system-overview.md)
5. [Trạng thái Matter node](architecture/matter-node.md)

### Onboarding kỹ thuật đầy đủ

1. [Bắt đầu](handbook/00-bat-dau.md)
2. [Sản phẩm và use case](handbook/01-san-pham-va-use-case.md)
3. [Kiến trúc toàn hệ thống](handbook/02-kien-truc-toan-he-thong.md)
4. [Phần cứng node](handbook/03-phan-cung-node.md)
5. [Firmware node](handbook/04-firmware-node.md)
6. [Matter và Thread](handbook/05-matter-thread.md)
7. [BBB Gateway và WebUI](handbook/06-bbb-gateway-webui.md)
8. [Build và phát triển](handbook/07-build-va-phat-trien.md)
9. [Flash, commissioning và deploy](handbook/08-flash-commission-deploy.md)
10. [Kiểm thử và bằng chứng](handbook/09-kiem-thu-va-bang-chung.md)
11. [Security, OTA và sản xuất](handbook/10-security-ota-san-xuat.md)
12. [Vận hành và xử lý sự cố](handbook/11-van-hanh-va-xu-ly-su-co.md)
13. [Glossary và ownership](handbook/12-glossary-va-ownership.md)

### Theo vai trò

| Vai trò | Nên đọc trước |
|---|---|
| Firmware | 03 → 04 → 05 → 07 → 09 |
| Gateway/backend | 02 → 05 → 06 → 08 → 11 |
| WebUI | 01 → 02 → 06 → 09 |
| QA/HIL | 01 → 03 → 05 → 08 → 09 → 11 |
| Operations | 02 → 06 → 08 → 10 → 11 |
| Hardware/security | 03 → 05 → 10 → 09 |

## Source of truth

Ưu tiên khi hai tài liệu mâu thuẫn:

1. Source, CMake, config, tests và hardware evidence hiện tại.
2. Handbook này và [architecture documents](architecture/).
3. [Architecture/coding/dependency rules](rules/).
4. Component README và [framework catalog](../external/agile-firmware-framework/docs/catalog.md).
5. Sibling repository `C:\Users\lesli\WS\agile-dashboard` cho WebUI/BBB as-built.
6. `docs/full-context/` là shared snapshot, không thay thế sibling source.
7. `reference/`, `Refactor_plan.md`, `.embedder/plans/` và build artifacts không phải source of truth.

## Tài liệu chi tiết hiện có

- [Kiến trúc toàn hệ thống end-to-end](architecture/system-overview.md)
- [Cấu trúc repository](architecture/repository-structure.md)
- [Matter node architecture](architecture/matter-node.md)
- [Local/Gateway/Cloud topology](architecture/local-gateway-cloud.md)
- [Active MQTT Gateway contract](full-context/03-contract-mqtt-gateway.md)
- [Node service catalog](architecture/node-service-catalog.md)
- [Architecture rules](rules/architecture.md)
- [Coding standards](rules/coding-standards.md)
- [Dependency rules](rules/dependencies.md)
- [Layer 5 review checklist](checklists/level5-change.md)
- [Hardware acceptance matrix](../tests/hil/README.md)
- [Board definition](../components/board_esp32c6/README.md)
- [Product component](../components/product_smart_device/README.md)

## Các codebase active và reference

| Repository/thư mục | Ownership |
|---|---|
| `agile-smart-device` | ESP32-C6 application node, reusable framework submodule, firmware tests và HIL contract. |
| `agile-dashboard` | WebUI, MQTT contracts, Gateway, matter.js Controller source, BBB deployment và as-built operations docs. |
| `mobileapp-reference/` | Ionic/Capacitor Mobile, typed client SDK, Android Keystore plugin và Mobile as-built context. |
| `dashboard-reference/` | Pinned snapshot của `agile-dashboard` để cross-reference architecture/provisioning; không tham gia firmware build. |

Các đường dẫn canonical của sibling repository:

```text
C:\Users\lesli\WS\agile-dashboard\docs\full-context\
C:\Users\lesli\WS\agile-dashboard\packages\contracts\src\
C:\Users\lesli\WS\agile-dashboard\packages\gateway\src\
C:\Users\lesli\WS\agile-dashboard\packages\matter-controller\src\
C:\Users\lesli\WS\agile-dashboard\apps\webui\src\
C:\Users\lesli\WS\agile-dashboard\deploy\
```

## Quy tắc cập nhật tài liệu

Khi thay đổi pin, profile, endpoint, cluster, MQTT contract, NVS schema, partition, toolchain hoặc deployment:

1. sửa source/config và test;
2. chạy gate phù hợp;
3. cập nhật chương handbook liên quan;
4. cập nhật tài liệu architecture/component authoritative;
5. ghi rõ phần nào Verified và phần nào chưa HIL;
6. không ghi credential, Thread dataset, passcode, DAC private key hoặc fabric secret vào Git.
