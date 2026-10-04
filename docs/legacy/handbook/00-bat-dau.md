# 00 — Bắt đầu với Agile Smart Device

> **LEGACY / NON-AUTHORITATIVE.** Handbook cũ được giữ để tra cứu. Bắt đầu tại [docs authoritative](../README.md).

## Mục tiêu

Sau chương này, kỹ sư mới biết repository nào cần mở, trạng thái thật của sản phẩm, cách chạy gate cơ bản và tài liệu nào cần đọc tiếp.

## Sản phẩm hiện tại

Sản phẩm là một ESP32-C6 application node điều khiển relay một kênh. Node hỗ trợ local button và được thiết kế thành Matter On/Off Plug-in Unit chạy trên Thread.

| Phần | Trạng thái |
|---|---|
| Local button → relay/LED | **Implemented và host-tested; cần physical HIL đầy đủ** |
| NVS state restore | **Implemented; power-cycle HIL còn mở** |
| Matter endpoint/OpenThread/CHIPoBLE | **Verified build, flash và boot** |
| BBB OTBR | **Verified active, role leader** |
| BBB matter.js Controller | **Service active; RPC thiếu commission/remove/read/subscription** |
| WebUI/MQTT/Gateway | **Deployed; Gateway production vẫn chạy mock khi chưa có node** |
| End-to-end WebUI → relay | **Pending** |
| Production security/OTA | **Production blocker** |

## Hai repository cần biết

```text
C:\Users\lesli\WS\agile-smart-device   firmware node
C:\Users\lesli\WS\agile-dashboard      WebUI, Gateway, Controller, BBB deploy
```

Không phát triển Gateway/WebUI trong `reference/` của firmware. `reference/` chỉ dùng đối chiếu legacy.

## Prerequisites

- Windows 10/11 x64.
- Git và recursive submodules.
- CMake/compiler host cho unit tests.
- ESP-IDF 6.0.2 cho `local_switch`/`gateway_node` compile gates.
- Ubuntu 24.04 WSL2 cho Matter.
- ESP-IDF v5.5.5 và ESP-Matter release/v1.6 đúng commit đã pin.
- ESP32-C6 board khi chạy hardware gates.
- SSH key tới BBB khi audit/deploy.

## Chạy kiểm tra nhanh

### Host tests

```cmd
cmake -S tests\host -B tests\host\build
cmake --build tests\host\build
ctest --test-dir tests\host\build --output-on-failure
```

### Framework tests

```cmd
cmake -S external\agile-firmware-framework -B external\agile-firmware-framework\build
cmake --build external\agile-firmware-framework\build
ctest --test-dir external\agile-firmware-framework\build --output-on-failure
```

### Local firmware

```cmd
call C:\Users\lesli\espv6\v6.0.2\esp-idf\export.bat
idf.py reconfigure -DPRODUCT_PROFILE=local_switch
idf.py build
```

### Matter firmware

```cmd
wsl -d Ubuntu-24.04 -u root -- bash /mnt/c/Users/lesli/WS/agile-smart-device/tools/build_matter_node.sh
```

Không chạy local và Matter ESP-IDF build đồng thời vì chúng dùng chung generated `managed_components/`.

## Không được làm

- Không commit passcode, password, Thread dataset, Network Key hoặc attestation private key.
- Không xóa `/var/lib/matter-controller` để chữa lỗi nếu chưa backup và hiểu hậu quả mất fabric.
- Không mở RCP serial từ Gateway; `otbr-agent` sở hữu độc quyền.
- Không gọi mock response là Matter HIL evidence.
- Không flash/erase board nếu chưa xác nhận đúng COM port/chip.
- Không coi `sdkconfig` root là Matter config; Matter dùng `sdkconfig.defaults.matter_node` và build directory riêng.

## Đọc tiếp

- Muốn hiểu product → [01](01-san-pham-va-use-case.md)
- Muốn hiểu topology → [02](02-kien-truc-toan-he-thong.md)
- Muốn sửa firmware → [04](04-firmware-node.md)
- Muốn build/flash → [07](07-build-va-phat-trien.md), [08](08-flash-commission-deploy.md)
- Muốn test → [09](09-kiem-thu-va-bang-chung.md)
