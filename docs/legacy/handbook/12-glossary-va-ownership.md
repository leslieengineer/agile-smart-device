# 12 — Glossary và ownership

## Glossary

| Thuật ngữ | Ý nghĩa trong dự án |
|---|---|
| Matter | Application/security protocol cho smart-home interoperability. |
| Thread | IPv6 low-power mesh trên IEEE 802.15.4. |
| OTBR | OpenThread Border Router nối Thread với IP backbone. |
| RCP | Radio Co-Processor, radio-only ESP32-C6 cho OTBR. |
| PASE | Passcode-authenticated commissioning session. |
| CASE | Certificate-authenticated operational session trong fabric. |
| Fabric | Matter administrative/security domain do Controller quản lý. |
| Node ID | 64-bit operational identity trong fabric. |
| Endpoint | Logical device/function trong Matter node. |
| Device type | Chuẩn mô tả tập cluster bắt buộc/optional của endpoint. |
| Cluster | Nhóm attributes/commands/events, ví dụ OnOff `0x0006`. |
| Attribute | State trong cluster, ví dụ OnOff boolean. |
| Command | Action Matter client invoke. |
| Subscription | Periodic/change-driven attribute/event reporting. |
| Commissioning | Đưa factory-new node vào network/fabric. |
| RCP Spinel | Protocol giữa `otbr-agent` và radio coprocessor. |
| UHAL | Platform-neutral hardware contracts của framework. |
| Composition root | Nơi tạo concrete object graph. |
| HIL | Hardware-in-the-loop acceptance test. |
| DAC/PAI/CD | Matter device attestation certificate chain/material. |
| LWT | MQTT Last Will and Testament. |

## Process ownership

| Process/component | Owner | Không được làm |
|---|---|---|
| WebUI | Frontend team | Không truy cập fabric/RCP trực tiếp. |
| Mosquitto | Platform/operations | Không cho anonymous hoặc bypass ACL. |
| Gateway | Backend/Gateway team | Không sở hữu physical relay state. |
| Matter Controller | Controller team | Không mở RCP serial. |
| OTBR | Platform/Thread team | Không xử lý WebUI JSON. |
| RCP | Thread platform | Không có Matter application endpoint. |
| Application node | Firmware/product team | Không implement MQTT/WebUI contract. |
| Board hardware | Hardware team | Không dựa firmware thay fail-safe design. |

## Repository ownership

| Path | Ownership |
|---|---|
| `main/` | ESP-IDF entry only. |
| `components/board_esp32c6/` | Board facts/wiring. |
| `components/product_smart_device/` | Layer 5 product. |
| `imports/` | Profile-selected ESP-IDF bridges. |
| `external/agile-firmware-framework/` | Reusable Layers 1–4. |
| `tests/` | Host/architecture/HIL contracts. |
| `tools/` | Build/check automation. |
| `docs/` | Product/architecture/rules/handbook. |
| `reference/` | Legacy/reference-only. |
| sibling `agile-dashboard` | WebUI/Gateway/Controller/deploy. |

## RACI đơn giản

| Hoạt động | Firmware | Gateway | WebUI | Ops | QA | Hardware/Security |
|---|---|---|---|---|---|---|
| Endpoint/cluster | R/A | C | I | I | C | C |
| MQTT contract | C | R/A | C | I | C | I |
| Controller API | C | R/A | I | C | C | C |
| OTBR/RCP | I | C | I | R/A | C | C |
| Relay/pin/safety | R | I | I | I | C | A |
| Commissioning UX | R | C | C | C | C | A |
| HIL/E2E | C | C | C | C | R/A | C |
| Attestation/OTA signing | C | C | I | R | C | A |

R = Responsible, A = Accountable, C = Consulted, I = Informed.

## Decision/open-issue links

- [Current status and decisions](../full-context/00-trang-thai-va-quyet-dinh.md)
- [Roadmap/open issues](../full-context/07-roadmap-va-open-issues.md)
- [Matter node gates](../architecture/matter-node.md)
- [Architecture rules](../rules/architecture.md)
- [HIL acceptance](../../tests/hil/README.md)
