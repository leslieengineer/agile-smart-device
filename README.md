# Agile Smart Device

Hệ thống Matter-over-Thread gồm firmware ESP32-C6 application node, Android commissioning app, BeagleBone Black OTBR/Matter Controller/Gateway/BFF và WebUI.

Bắt đầu tại [Tài liệu hệ thống authoritative](docs/README.md).

## Trạng thái

| Hạng mục | Source | Deployed | HIL |
|---|---|---|---|
| ESP32-C6 On/Off node | Có | Có | Từng phần |
| Rhophi BLE claim | Có | Có | Từng bước |
| Android Matter commissioning | Có | Có | Đang debug final flow |
| BBB OTBR/Matter Controller | Có | Có | RPC/Thread từng phần |
| WebUI dynamic inventory | Có | Có | Chưa với permanent node cuối |
| Production security/release | Một phần | Không | Không |

Xem [status và open issues](docs/12-status-open-issues.md) để biết trạng thái chi tiết. Build pass không đồng nghĩa HIL pass.

## Hardware node

| Chức năng | Resource |
|---|---:|
| Button active-low | GPIO9 |
| Relay active-high | GPIO10 |
| Relay LED | GPIO2 |
| WS2812 status | GPIO8 |
| Flash | 16 MiB, dual OTA layout cho Matter |

## Quick start

```bash
python3 tools/check_layer_boundaries.py
cmake -S tests/host -B build/host-tests -G Ninja
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
bash tools/build_matter_node.sh
```

Flash/deploy/erase là thao tác thay đổi trạng thái; dùng [runbook](docs/runbooks/README.md) thay vì copy lệnh từ tài liệu legacy.

## Repository

- `components/` — board và product firmware.
- `external/agile-firmware-framework/` — reusable framework submodule.
- `dashboard-reference/` — BBB/WebUI/controller source.
- `mobileapp-reference/` — mobile app, SDK và native plugin.
- `tests/` — host, architecture và HIL contract.
- `tools/` — build, flash, manufacturing và validation.
- `docs/` — tài liệu authoritative.

## Quy tắc

Đọc `AGENTS.md`, `docs/rules/` và checklist trước khi sửa firmware. Không đưa Thread dataset, claim secret, registry key, passcode, DAC private key, bearer token hoặc password vào Git/log/tài liệu.