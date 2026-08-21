# Tài liệu hệ thống Agile Smart Device

Cập nhật: 2026-08-19. Đây là bộ tài liệu **authoritative** của hệ thống. Source, config, schema và test có quyền ưu tiên cao hơn tài liệu khi có mâu thuẫn.

## Nhãn trạng thái

| Nhãn | Ý nghĩa |
|---|---|
| Source | Hành vi có trong working tree và có gate phù hợp. |
| Deployed | Artifact đã được build/cài/flash và health check trên target thật. |
| HIL | Luồng đã pass end-to-end với evidence đã redact. |
| Đang debug | Đã triển khai một phần nhưng acceptance cuối chưa đạt. |

Không suy diễn `HIL` từ việc code compile hoặc service đang active. Không tuyên bố production-ready hay compliance nếu thiếu bằng chứng.

## Chương authoritative

1. [Tổng quan sản phẩm](00-tong-quan-san-pham.md)
2. [Kiến trúc end-to-end](01-kien-truc-end-to-end.md)
3. [Firmware ESP32-C6](02-firmware-esp32c6.md)
4. [Rhophi claim GATT](03-rhophi-claim-gatt.md)
5. [Mobile commissioning](04-mobile-commissioning.md)
6. [BBB Gateway, Controller và WebUI](05-bbb-gateway-controller-webui.md)
7. [Thread và IPv6 routing](06-thread-ipv6-routing.md)
8. [REST, MQTT và SSE contracts](07-contract-api-mqtt-sse.md)
9. [Build, flash và deploy](08-build-flash-deploy.md)
10. [Security, manufacturing và secrets](09-security-manufacturing-secrets.md)
11. [Testing, HIL và evidence](10-testing-hil-evidence.md)
12. [Vận hành và troubleshooting](11-operations-troubleshooting.md)
13. [Trạng thái và open issues](12-status-open-issues.md)
14. [Thuật ngữ, ownership và repository](13-thuat-ngu-ownership-repository.md)

## Lộ trình theo vai trò

| Vai trò | Thứ tự đọc |
|---|---|
| Firmware | 02 → 03 → 08 → 10 → 11 |
| Mobile | 04 → 03 → 06 → 07 → 11 |
| BBB/backend | 05 → 06 → 07 → 08 → 11 |
| WebUI | 01 → 05 → 07 → 10 |
| QA/HIL | 00 → 04 → 05 → 10 → 12 |
| Operations | 01 → 05 → 06 → 08 → 11 |

## Bốn tài liệu tiêu chuẩn

- [Danh mục bốn tài liệu](guides/README.md)
- [ESP32-C6 trên Ubuntu 24.04](guides/01-esp32c6-ubuntu-24.04.md)
- [Mobile App trên Ubuntu 24.04](guides/02-mobile-app-ubuntu-24.04.md)
- [WebUI trên Ubuntu 24.04](guides/03-webui-ubuntu-24.04.md)
- [BeagleBone Black Debian 11 production](guides/05-bbb-debian11-production-dashboard.md)
- [Phụ lục lệnh BeagleBone Black](guides/04-beaglebone-black-operations.md)

## Runbook

- [Danh mục runbook](runbooks/README.md)
- [Bench lab](runbooks/lab-hardware.md)
- [Firmware build/flash](runbooks/firmware-build-flash.md)
- [Android build/install](runbooks/android-build-install.md)
- [BBB deploy/rollback](runbooks/bbb-deploy-rollback.md)
- [Commissioning end-to-end](runbooks/commissioning-e2e.md)
- [Recovery/reset](runbooks/recovery-reset.md)

## Tài liệu khác

- [Quy tắc kiến trúc](rules/architecture.md)
- [Coding standard](rules/coding-standards.md)
- [Dependency policy](rules/dependencies.md)
- [Checklist Layer 5](checklists/level5-change.md)
- [Checklist tài liệu](checklists/doc-change.md)
- [Legacy index](legacy/README.md)

`docs/handbook`, `docs/full-context`, `docs/architecture`, course trong các reference repository và các kế hoạch cũ chỉ dùng tra cứu lịch sử.