# 07 — Build và phát triển

## Profile matrix

| Profile | SDK | Mục đích |
|---|---|---|
| `local_switch` | ESP-IDF 6.0.2 | Local regression firmware, profile mặc định. |
| `gateway_node` | ESP-IDF 6.0.2 | Compile-only Layer 4 catalog; không phải production Gateway/node. |
| `matter_node` | ESP-IDF v5.5.5 + ESP-Matter v1.6 | Production-oriented Matter-over-Thread node. |

## Repository setup

```cmd
git submodule update --init --recursive
```

Framework và `reference` là submodules. Không chỉnh gitlink/commit nếu chưa có change ownership rõ.

## Host tests

```cmd
cmake -S tests\host -B tests\host\build
cmake --build tests\host\build
ctest --test-dir tests\host\build --output-on-failure
```

Framework:

```cmd
cmake -S external\agile-firmware-framework -B external\agile-firmware-framework\build
cmake --build external\agile-firmware-framework\build
ctest --test-dir external\agile-firmware-framework\build --output-on-failure
```

## Local profile

```cmd
call C:\Users\lesli\espv6\v6.0.2\esp-idf\export.bat
idf.py set-target esp32c6
idf.py reconfigure -DPRODUCT_PROFILE=local_switch
idf.py build
idf.py size
```

`gateway_node` compile-only:

```cmd
idf.py reconfigure -DPRODUCT_PROFILE=gateway_node
idf.py build
```

Restore local profile sau audit nếu dùng chung build directory.

## Matter host setup

Official Matter flow dùng Linux. Windows host hiện dùng Ubuntu 24.04 WSL2.

Pinned paths mặc định của scripts:

```text
/opt/esp/esp-idf
/opt/esp/esp-matter
```

Pinned SHAs xem [Matter chapter](05-matter-thread.md).

### Upstream reference

```cmd
wsl -d Ubuntu-24.04 -u root -- bash /mnt/c/Users/lesli/WS/agile-smart-device/tools/build_matter_reference.sh
```

Reference phải pass trước khi quy lỗi cho product integration.

### Product Matter build

```cmd
wsl -d Ubuntu-24.04 -u root -- bash /mnt/c/Users/lesli/WS/agile-smart-device/tools/build_matter_node.sh
```

Script:

- verify exact IDF/ESP-Matter SHAs;
- source đúng environments;
- dùng `sdkconfig.defaults.matter_node`;
- dùng WSL build directory riêng;
- reconfigure để restore Matter managed dependencies;
- build `PRODUCT_PROFILE=matter_node`.

## Shared managed components constraint

Root source dùng generated `managed_components/`. Local IDF6 và Matter IDF5.5 resolve dependency sets khác nhau. Không chạy hai build song song. Matter wrapper luôn reconfigure; local build cũng cần reconfigure khi chuyển lại profile.

Không chỉnh tay `managed_components` hoặc `dependencies.lock`.

## Config và partitions

| File | Scope |
|---|---|
| `sdkconfig.defaults` | Local IDF6 default. |
| `sdkconfig` | Generated local working config. |
| `sdkconfig.defaults.matter_node` | Matter clean-build defaults. |
| `partitions_matter.csv` | 16 MiB dual OTA layout. |

## Compile database và binary audit

- Local: `build/compile_commands.json`.
- Matter: WSL build directory `compile_commands.json`.
- Kiểm tra component inventory trong `project_description.json`.
- Matter profile không được có `framework_mqtt*`.
- Binary không được chứa `home/control` hoặc `request_id`.
- Ghi size/map delta cho mỗi feature lớn.

## Quy tắc code

- C++17, exceptions/RTTI disabled trong firmware runtime.
- Không dynamic/unbounded allocation trong reusable Layer 4.
- Explicit CMake `SRCS`; không blanket glob.
- Vendor headers chỉ Layer 1/3 hoặc product infrastructure.
- Application và Layer 4 vendor-neutral.
- Constructor injection; không service locator/singleton API.
- Direct call cho synchronous flow; EventBus chỉ async multi-subscriber.

## Quy trình thay đổi

1. Xác định ownership/layer.
2. Viết/điều chỉnh host test trước khi target integration.
3. Sửa explicit CMake dependencies.
4. Chạy framework/parent tests.
5. Build local profile.
6. Build clean Matter profile nếu chạm product/platform.
7. Kiểm tra size/compile inventory.
8. Chạy HIL đúng scope.
9. Cập nhật handbook/component/architecture docs.

Xem [coding rules](../rules/coding-standards.md) và [Layer 5 checklist](../checklists/level5-change.md).
