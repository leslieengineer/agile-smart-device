# Firmware build và flash

## Tiền điều kiện

- Checkout ESP-IDF/ESP-Matter đúng SHA trong `tools/build_matter_node.sh`.
- `fctry` image/registry được tạo ngoài worktree nếu provisioning material thay đổi.
- Xác minh ESP port bằng MAC, không chỉ dựa vào tên `/dev/ttyACM*`.

## Build

```bash
cd "${PROJECT_DIR}"
python3 tools/check_layer_boundaries.py
cmake -S tests/host -B build/host-tests -G Ninja
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
bash tools/build_matter_node.sh
```

## Flash app

Thao tác này thay firmware nhưng không nên ghi đè `fctry`.

```bash
PORT="${ESP_PORT}" bash tools/flash_matter_node.sh
```

Nếu dùng esptool trực tiếp, lấy offset từ generated `flash_args`, không chép offset từ tài liệu cũ.

## Factory partition

Chỉ flash `fctry` khi registry BBB tương ứng đã sẵn sàng. Backup partition cũ trước và không log nội dung.

## Verify

Monitor 115200 và kiểm tra các marker:

```text
Rhophi claim service ready
On/Off Plug-in Unit created on endpoint 1
Commissioning window opened
Rhophi claim window opened
```

## Rollback

Giữ previous application bin/ELF và factory backup ngoài Git. Không rollback factory partition khi chỉ rollback app.