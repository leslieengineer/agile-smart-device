# Tiêu chuẩn 1 — ESP32-C6 trên Ubuntu 24.04

Tài liệu này hướng dẫn từ máy Ubuntu 24.04 mới đến lúc build, test, flash và theo dõi firmware Matter-over-Thread của `agile-smart-device`.

## 1. Phạm vi và biến môi trường

```bash
export PROJECT_DIR="$HOME/WS/agile-smart-device"
export IDF_PATH="$HOME/esp/v6.0.2/esp-idf"
export ESP_MATTER_PATH="$HOME/esp-matter"
export ESP_PORT="/dev/ttyACM1"
```

Ý nghĩa:

- `export` tạo biến cho shell hiện tại và các process con.
- `PROJECT_DIR` là repository firmware.
- `IDF_PATH` là checkout ESP-IDF được script kiểm tra SHA.
- `ESP_MATTER_PATH` là checkout ESP-Matter.
- `ESP_PORT` là USB Serial/JTAG của ESP32-C6; phải xác minh trước khi flash.

Kiểm tra biến:

```bash
printf 'PROJECT_DIR=%s\nIDF_PATH=%s\nESP_MATTER_PATH=%s\nESP_PORT=%s\n' \
  "$PROJECT_DIR" "$IDF_PATH" "$ESP_MATTER_PATH" "$ESP_PORT"
```

`printf` in giá trị mà không thay đổi hệ thống. Dấu `\` nối một lệnh qua nhiều dòng.

## 2. Cài package hệ thống

```bash
sudo apt update
sudo apt install -y git cmake ninja-build ccache flex bison gperf \
  python3 python3-pip python3-venv libffi-dev libssl-dev \
  libusb-1.0-0 udev pkg-config wget curl unzip
```

Ý nghĩa:

- `sudo apt update` cập nhật package index, không nâng cấp package.
- `apt install` cài compiler support và công cụ build.
- `-y` tự xác nhận prompt của APT.
- `cmake` tạo build graph; `ninja-build` thực thi graph.
- `flex`, `bison`, `gperf` phục vụ code generation ESP-IDF.
- `python3-venv` cho môi trường Python riêng của ESP-IDF.
- `libusb`/`udev` phục vụ USB/JTAG/serial.

Kiểm tra:

```bash
git --version
cmake --version
ninja --version
python3 --version
```

Mỗi lệnh chỉ in version; dùng để xác nhận PATH trước khi clone.

## 3. Lấy ESP-IDF đúng commit

```bash
mkdir -p "$HOME/esp/v6.0.2"
git clone --recursive https://github.com/espressif/esp-idf.git "$IDF_PATH"
git -C "$IDF_PATH" checkout 662a3be354759d9487bf4b1a629fadb766cb1800
git -C "$IDF_PATH" submodule update --init --recursive
```

Ý nghĩa:

- `mkdir -p` tạo cả parent directory và không lỗi nếu đã tồn tại.
- `git clone --recursive` lấy repository cùng submodule ban đầu.
- `git -C <dir>` chạy Git trong directory mà không cần `cd`.
- `checkout <sha>` pin source chính xác; không dùng branch nổi.
- `submodule update --init --recursive` đồng bộ mọi dependency nested.

Cài toolchain cho ESP32-C6:

```bash
"$IDF_PATH/install.sh" esp32c6
```

Script cài compiler RISC-V, debugger, CMake helpers và Python virtual environment cho target `esp32c6`.

Kích hoạt ESP-IDF trong shell mới:

```bash
source "$IDF_PATH/export.sh"
```

`source` chạy script trong shell hiện tại để thêm `idf.py`, compiler và Python environment vào PATH. Cần chạy lại sau khi mở terminal mới.

Xác minh pin:

```bash
git -C "$IDF_PATH" rev-parse HEAD
idf.py --version
```

`rev-parse HEAD` phải trả đúng SHA được pin trong `tools/build_matter_node.sh`.

## 4. Lấy ESP-Matter đúng commit

```bash
git clone --recursive https://github.com/espressif/esp-matter.git "$ESP_MATTER_PATH"
git -C "$ESP_MATTER_PATH" checkout 881a8ff5cada10a197481d8e332bf41347702c27
git -C "$ESP_MATTER_PATH" submodule update --init --recursive
```

Các tham số Git có cùng ý nghĩa phần ESP-IDF. ESP-Matter chứa ConnectedHomeIP nên `--recursive` là bắt buộc.

Kích hoạt:

```bash
source "$ESP_MATTER_PATH/export.sh"
```

Script thêm ESP-Matter component paths và Matter build tools vào shell.

Xác minh:

```bash
git -C "$ESP_MATTER_PATH" rev-parse HEAD
git -C "$ESP_MATTER_PATH/connectedhomeip/connectedhomeip" rev-parse HEAD
```

Lệnh thứ hai kiểm tra ConnectedHomeIP thực tế mà ESP-Matter sử dụng.

## 5. Lấy source dự án

```bash
mkdir -p "$HOME/WS"
git clone --recurse-submodules <PROJECT_GIT_URL> "$PROJECT_DIR"
git -C "$PROJECT_DIR" submodule update --init --recursive
```

Thay `<PROJECT_GIT_URL>` bằng URL repository thật. Không ghi credential vào command.

Nếu repository đã có:

```bash
git -C "$PROJECT_DIR" status --short
git -C "$PROJECT_DIR" submodule status
```

- `status --short` cho biết file đang sửa/untracked.
- `submodule status` cho biết commit của framework/dashboard/mobile/reference.

Không reset hoặc checkout đè khi working tree có thay đổi chưa lưu.

## 6. Xác minh hardware và permission

Liệt kê port:

```bash
ls -l /dev/ttyACM* /dev/serial/by-id/ 2>/dev/null
```

- `ls -l` in device, owner, group và symlink.
- `2>/dev/null` ẩn lỗi của pattern không tồn tại.
- Ưu tiên `/dev/serial/by-id/...` trong automation vì ổn định hơn `ttyACM1`.

Thêm user vào group serial:

```bash
sudo usermod -aG dialout "$USER"
```

- `usermod` sửa group membership.
- `-aG` append group, không xóa group cũ.
- Logout/login lại để group mới có hiệu lực.

Đọc MAC, không ghi flash:

```bash
source "$IDF_PATH/export.sh"
python -m esptool --port "$ESP_PORT" read-mac
```

- `python -m esptool` dùng esptool trong IDF environment.
- `--port` chọn đúng board.
- `read-mac` chỉ đọc chip identity và reset board khi hoàn tất.

## 7. Kiểm tra kiến trúc và host tests

```bash
cd "$PROJECT_DIR"
python3 tools/check_layer_boundaries.py
```

Checker xác nhận dependency direction và vendor header không leak vào layer cấm.

Configure host tests:

```bash
cmake -S tests/host -B build/host-tests -G Ninja
```

- `-S tests/host` chọn source CMake.
- `-B build/host-tests` chọn build directory riêng.
- `-G Ninja` dùng Ninja generator.

Build:

```bash
cmake --build build/host-tests
```

CMake gọi Ninja và chỉ rebuild file thay đổi.

Run tests:

```bash
ctest --test-dir build/host-tests --output-on-failure
```

- `--test-dir` chạy test trong build directory.
- `--output-on-failure` giữ output gọn khi pass nhưng in chi tiết khi fail.

Expected: application, claim protocol, architecture và manufacturing tests pass.

## 8. Build Matter firmware

```bash
cd "$PROJECT_DIR"
bash tools/build_matter_node.sh
```

Script thực hiện:

1. Kiểm tra SHA ESP-IDF/ESP-Matter.
2. Source hai environment.
3. Chọn `sdkconfig.defaults.matter_node`.
4. Chọn profile `matter_node`.
5. Tắt `RHOPHI_CLAIM_DEV_BYPASS` mặc định.
6. `set-target esp32c6` ở build đầu hoặc `reconfigure build` ở build sau.

Artifact chính:

```bash
ls -lh build-matter/agile_smart_device.bin \
  build-matter/agile_smart_device.elf \
  build-matter/partition_table/partition-table.bin
```

- `.bin` dùng flash.
- `.elf` dùng symbol/debug/backtrace.
- partition table xác định offset và kích thước partition.

Kiểm tra app size:

```bash
source "$IDF_PATH/export.sh"
idf.py -B build-matter size
```

`-B` chọn build directory; `size` phân tích flash/DRAM/IRAM mà không rebuild toàn bộ.

## 9. Build sạch và reconfigure

Reconfigure sau khi đổi CMake/Kconfig:

```bash
source "$IDF_PATH/export.sh"
idf.py -B build-matter reconfigure
```

Xóa build generated rồi build lại:

```bash
rm -rf build-matter
bash tools/build_matter_node.sh
```

`rm -rf` là destructive với build artifact nhưng không xóa source/NVS board. Chỉ dùng khi cache thật sự lỗi.

## 10. Manufacturing material

Tạo material ngoài repository:

```bash
python3 tools/mfg/generate_device.py \
  --serial <DEVICE_SERIAL> \
  --chip-mac <BASE_MAC> \
  --product-id 1 \
  --setup-passcode <LAB_PASSCODE> \
  --discriminator <LAB_DISCRIMINATOR> \
  --out "$HOME/private/rhophi-lab"
```

- `--serial` là device identity vận hành.
- `--chip-mac` bind claim ID với chip.
- `--product-id` phải khớp registry/product.
- setup value là Matter onboarding data; production phải unique.
- `--out` bắt buộc ngoài Git worktree.

Không in hoặc commit file sinh ra.

Tạo factory NVS image:

```bash
source "$IDF_PATH/export.sh"
python tools/mfg/build_factory_partition.py \
  --csv "$HOME/private/rhophi-lab/<DEVICE>.fctry.csv" \
  --out "$HOME/private/rhophi-lab/fctry.bin" \
  --size 0x6000
```

`--size` phải khớp `fctry` trong `partitions_matter.csv`.

## 11. Flash firmware

Cách chuẩn qua wrapper:

```bash
cd "$PROJECT_DIR"
PORT="$ESP_PORT" bash tools/flash_matter_node.sh
```

- `PORT=...` override port cho đúng command này.
- Wrapper source toolchain rồi gọi `idf.py flash` với build directory.
- Flash app/bootloader/partition table; không tự đảm bảo factory image được cập nhật.

Flash app-only khi partition table/bootloader không đổi:

```bash
source "$IDF_PATH/export.sh"
python -m esptool --chip esp32c6 --port "$ESP_PORT" \
  --baud 460800 write-flash 0x20000 build-matter/agile_smart_device.bin
```

- `--chip esp32c6` ngăn auto-detect sai.
- `--baud 460800` tăng tốc; hạ 115200 nếu USB không ổn định.
- `write-flash 0x20000` ghi app vào offset hiện tại. Luôn đối chiếu `build-matter/flash_args`; không dùng offset này cho partition table khác.

Factory partition là thao tác riêng và phải có registry BBB tương ứng. Backup trước khi ghi.

## 12. Serial monitor và run

```bash
source "$IDF_PATH/export.sh"
idf.py -p "$ESP_PORT" -B build-matter monitor
```

- `-p` chọn port.
- `-B` dùng ELF/build metadata để decode panic.
- `monitor` mở UART 115200 và decode address.
- Thoát bằng `Ctrl+]`.

Marker boot đúng:

```text
Rhophi claim service ready
On/Off Plug-in Unit created on endpoint 1
Commissioning window opened
Rhophi claim window opened
```

## 13. Kiểm tra sau flash

1. Short press đổi relay và GPIO2 LED đúng một lần.
2. App scan thấy Matter advertiser.
3. Identity GATT có 36 byte và claimable flag.
4. OTBR thấy node sau Thread provisioning.
5. Không có watchdog, stack overflow, NVS corrupt hoặc repeated reboot.

## 14. Lỗi thường gặp

### `ESP-IDF must be pinned`

```bash
git -C "$IDF_PATH" rev-parse HEAD
git -C "$ESP_MATTER_PATH" rev-parse HEAD
```

Nếu SHA khác, dùng checkout đúng; không sửa expected SHA chỉ để build qua.

### `Permission denied /dev/ttyACM*`

Kiểm tra `groups`, thêm `dialout`, logout/login và đóng serial monitor khác.

### Esptool timeout

Hạ baud, dùng cáp data tốt, đóng monitor và xác minh board không bị giữ GPIO9 lúc reset.

### Claim window không mở

Kiểm tra boot log `Rhophi claim window opened`; firmware chỉ auto-open khi fabric count bằng 0.

### Không xóa được stale fabric

Không erase `fctry`. Dùng factory reset Matter hoặc recovery runbook để chỉ xóa default Matter NVS.

## 15. Apple Home và Google Home

### Node hiện tại có add được chưa?

**Chưa ở mức consumer-ready.** Build hiện tại có Matter/Thread/OnOff đúng nền tảng nhưng dùng test identity và example attestation.

Kiểm tra config đã build:

```bash
grep -E 'CONFIG_DEVICE_(VENDOR|PRODUCT)_ID|CONFIG_ENABLE_TEST_SETUP_PARAMS|CONFIG_EXAMPLE_DAC_PROVIDER|CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER' \
  build-matter/sdkconfig
```

Ý nghĩa:

- `grep -E` tìm nhiều pattern bằng extended regular expression.
- `CONFIG_DEVICE_VENDOR_ID=0xFFF1` là test VID.
- `CONFIG_DEVICE_PRODUCT_ID=0x8000` là PID development hiện tại.
- `ENABLE_TEST_SETUP_PARAMS=y` nghĩa setup data chưa phải unique production data.
- `EXAMPLE_DAC_PROVIDER=y` nghĩa Device Attestation Certificate chỉ dùng development.
- Factory Data Provider chưa bật.

Do đó:

- Google Home developer/Field Trial có thể được thử sau khi tạo Developer Console integration khớp VID/PID và có QR/manual onboarding code.
- Google Home consumer chưa được hỗ trợ cho đến CSA/Google certification.
- Apple Home development có thể thử với QR/manual code và Apple Thread Border Router, nhưng chưa có evidence pass.
- Works with Apple Home/Google Home badge chưa được phép dùng.

### Prerequisite cho direct ecosystem pairing

1. Node factory-new, Matter commissioning window mở.
2. QR/manual setup payload đúng passcode/discriminator/VID/PID.
3. Google Nest hoặc Apple HomePod/Apple TV làm Thread Border Router tương ứng.
4. Development account/project/integration theo ecosystem.
5. Attestation policy chấp nhận development credentials.

### Multi-Admin path đề xuất

1. Commission node vào Rhophi/BBB.
2. Mobile/WebUI yêu cầu BBB mở Enhanced Commissioning Window.
3. Sinh QR/manual code chỉ dùng một lần.
4. Google Home hoặc Apple Home scan code và AddNOC fabric thứ hai.
5. Xác nhận node vẫn thuộc BBB fabric và ecosystem fabric.
6. Test On/Off từ cả hai hệ sinh thái.

Không chia sẻ Rhophi claim secret hoặc Thread dataset; ecosystem thứ hai chỉ nhận standard Matter onboarding payload.

## 16. Roadmap node

### Mốc A — HIL ổn định

- Permanent BBB fabric, temporary fabric cleanup.
- Inventory/OnOff hai chiều.
- Restart/remove/recommission.
- Regression automation.

### Mốc B — Ecosystem development

- QR/manual onboarding payload.
- Unique factory passcode/discriminator.
- Basic Information hoàn chỉnh.
- Google Developer Console test integration.
- Apple Home/Google Home development HIL.
- Multi-Admin share/revoke UX.

### Mốc C — Production

- CSA VID/PID và certification.
- DAC/PAI/CD production.
- Secure boot, flash encryption, anti-rollback.
- Signed Matter OTA.
- Diagnostics clusters và field logs.
- Google Home certification/launch và Apple interoperability/badge approval.

### Mốc D — Các SKU mới

- Multi-gang OnOff.
- Dimmer/LevelControl.
- Energy-monitoring plug.
- Window Covering motor node.
- Temperature/humidity/air-quality sensor.
- Motion/contact sensor.
- Battery Sleepy End Device/ICD.
- Matter bridge cho thiết bị legacy.

Mỗi SKU cần ProductProfile, endpoint model, manufacturing schema, resource budget và HIL matrix riêng.

## 17. Lệnh validation cuối

```bash
python3 tools/check_layer_boundaries.py
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
bash tools/build_matter_node.sh
python3 tools/check_docs.py
```

Chỉ đánh HIL pass sau commissioning, permanent fabric, inventory và On/Off hai chiều; build pass chưa đủ.