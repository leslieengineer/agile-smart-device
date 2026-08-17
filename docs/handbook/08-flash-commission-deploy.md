# 08 — Flash, commissioning và deploy runbook

## Safety gate

Trước hardware action:

- xác nhận board/SKU;
- xác nhận đúng COM port;
- lưu firmware/toolchain SHA;
- không giữ GPIO9 khi reset trừ khi cần download mode;
- hiểu flash sẽ thay partition/application hiện tại;
- không erase fabric/NVS production khi chưa backup.

## Nhận diện chip

Dùng esptool của toolchain phù hợp:

```cmd
python -m esptool --chip esp32c6 --port COMx chip_id
```

Kỳ vọng chip ESP32-C6 và flash/USB mode phù hợp. Nếu không connect, xem [troubleshooting](11-van-hanh-va-xu-ly-su-co.md).

## Flash local profile

```cmd
call C:\Users\lesli\espv6\v6.0.2\esp-idf\export.bat
idf.py -p COMx flash
```

Local layout không giống Matter dual-OTA layout. Khi chuyển profile phải flash partition table cùng firmware.

## Flash Matter profile

1. Chạy clean Matter build bằng [wrapper](07-build-va-phat-trien.md).
2. Dùng `flash_args`/command được chính build output sinh ra.
3. Flash bootloader, partition table, initial OTA data và application image.
4. Monitor từ reset để lấy boot evidence.

Không hard-code offset từ tài liệu nếu partition table đã thay đổi; lấy offset từ `partitions_matter.csv`/build output.

## Boot log checklist

Kỳ vọng:

- project/IDF version;
- ESP32-C6 revision;
- Matter On/Off Plug-in Unit endpoint;
- reset reason/free heap;
- OpenThread started;
- Matter server listening;
- CHIPoBLE advertising/window open;
- không crash, watchdog, stack overflow hoặc NVS fatal.

mDNS advertise có thể lỗi trước khi node có Thread credentials; phải phân biệt uncommissioned state với runtime failure.

## BBB preflight

Read-only checks:

```text
systemctl is-active otbr-agent
ip link show wpan0
ip -6 route show
sudo ot-ctl state
sudo ot-ctl child table
systemctl is-active matter-controller
systemctl is-active matter-gateway
systemctl is-active mosquitto
```

Không chạy command in Thread dataset/key vào shared logs.

## Commissioning

### Trạng thái hiện tại

**Blocked cho final product flow.** BBB matter.js service hiện chưa expose `commission`, `removeNode`, explicit `read` và subscription streaming qua RPC. Không có lệnh commissioning production hợp lệ trong repository firmware.

Commissioning chỉ được coi pass khi API/source tại sibling `agile-dashboard/packages/matter-controller` được hoàn thiện, build/deploy và test.

### Flow yêu cầu sau khi API hoàn chỉnh

1. Factory reset node có chủ đích.
2. Mở commissioning window.
3. Controller nhận setup payload qua secure operator boundary.
4. BLE discovery/PASE.
5. Attestation decision.
6. Controller cấp active Thread dataset mà không log secret.
7. Node attach `wpan0` network.
8. Fabric credentials/CASE.
9. Persist Node ID, Descriptor và endpoint inventory.
10. Read OnOff và subscribe.

### Acceptance sau commission

- OTBR child/router table phản ánh node.
- Controller listNodes có operational Node ID.
- Descriptor có On/Off Plug-in Unit.
- On/Off read bằng physical state.
- On/Off/Toggle invoke thay đổi relay.
- Local press tạo subscription report.

## Remove/recommission

- Ưu tiên decommission qua Controller, không xóa storage trực tiếp.
- Xác nhận node bị remove khỏi fabric/inventory.
- Factory reset node khi policy yêu cầu.
- Recommission và kiểm tra không duplicate inventory/subscription.

## Factory reset local gesture

Released hold ≥10 giây gọi Matter factory reset. GPIO9 strapping khiến gesture lúc reset nguy hiểm. Visual confirmation/pattern cuối cùng chưa HIL-approved; không dùng như production UX trước acceptance.

## Deploy BBB

Canonical deploy scripts nằm trong sibling `agile-dashboard/deploy`. Quy trình:

1. build/test source;
2. bundle artifact;
3. backup controller storage/config;
4. install bằng reviewed script;
5. `systemctl daemon-reload` nếu unit đổi;
6. restart dependency order Controller → Gateway;
7. health/log verification;
8. rollback artifact nếu health fail.

Không patch trực tiếp bundled `/opt/matter-controller/matter-controller.cjs` nếu source chưa được cập nhật.
