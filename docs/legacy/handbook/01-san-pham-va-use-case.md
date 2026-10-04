# 01 — Sản phẩm và use case

## Product definition

Agile Smart Device là node actuator trong hệ thống nhà thông minh. SKU đầu tiên điều khiển một tải điện bằng relay và được biểu diễn trong Matter dưới device type **On/Off Plug-in Unit**.

Node không phải Gateway, không phải OTBR và không phải RCP. Node không biết MQTT topic, WebUI JSON hoặc `request_id`.

## Giá trị sản phẩm

- Điều khiển tải tại chỗ ngay cả khi mạng lỗi.
- Điều khiển từ WebUI qua Matter/Thread khi hệ thống hoàn chỉnh.
- Đồng bộ trạng thái local/remote bằng một state owner.
- Khôi phục trạng thái relay hợp lệ sau reboot theo product policy.
- Có commissioning/factory-reset UX và status indication riêng.

## Actors

| Actor | Tương tác |
|---|---|
| Người dùng tại thiết bị | Nhấn GPIO9 để toggle; giữ để commissioning/factory reset. |
| Người dùng WebUI | Gửi On/Off/Toggle qua MQTT Gateway contract. |
| BBB Gateway | Validate Web contract và chuyển thành Matter Interaction Model. |
| Matter Controller | Sở hữu fabric, CASE session, invoke/read/subscription. |
| OTBR | Chuyển IPv6 giữa Ethernet và Thread. |
| Firmware node | Sở hữu relay state, hardware và Matter server endpoint. |
| QA/operations | Flash, commission, giám sát logs và chạy acceptance matrix. |

## Use case chính

### UC-01 — Local toggle

1. Người dùng nhấn ngắn GPIO9.
2. ISR chỉ enqueue event.
3. Runtime debounce và phát semantic short press.
4. Application toggle state.
5. Relay GPIO10 và LED GPIO2 thay đổi.
6. NVS write được coalesce.
7. Nếu Matter hoạt động, OnOff attribute được report.

Local path không phụ thuộc BBB, Thread hoặc WebUI.

### UC-02 — Remote command

1. WebUI publish MQTT request.
2. Gateway validate và map cluster/command.
3. Controller invoke Matter OnOff.
4. Node PRE_UPDATE enqueue remote command.
5. Product runtime áp dụng state.
6. Matter attribute và WebUI response/event phản ánh state thực tế.

Bước 3–6 chưa đạt final end-to-end gate vì Controller RPC còn thiếu commissioning/subscription.

### UC-03 — Power restore

- GPIO relay phải ở safe OFF trước khi firmware cấu hình.
- Firmware đọc NVS schema/state.
- State hợp lệ được apply; missing/corrupt state mặc định OFF.
- Matter attribute ban đầu phải khớp physical output.

### UC-04 — Commissioning

- Factory-new node quảng bá CHIPoBLE.
- Commissioner thực hiện PASE, attestation, cấp Thread credentials và operational credentials.
- Sau CASE, node có Node ID theo fabric.

### UC-05 — Recovery/factory reset

- Released hold ≥5 giây yêu cầu commissioning window.
- Released hold ≥10 giây yêu cầu Matter factory reset.
- Product relay-state namespace hiện được giữ lại theo policy.
- Visual confirmation trước hành động destructive vẫn cần HIL hoàn thiện.

## Degraded behavior

| Lỗi | Hành vi yêu cầu |
|---|---|
| WebUI/MQTT/Gateway down | Local button vẫn hoạt động. |
| Controller down | Relay giữ state; local control tiếp tục. |
| OTBR/Thread down | Không reboot loop; local control tiếp tục. |
| NVS state missing | Boot OFF. |
| NVS commit lỗi | Hardware state đã apply không rollback; ghi diagnostic. |
| Matter queue đầy | Reject command, không silent drop. |
| WS2812 lỗi | Matter/local control vẫn khởi động, log warning. |

## Out of scope hiện tại

- Dimmer/LevelControl hardware.
- Window covering motor, calibration và obstruction.
- Cooktop production safety.
- Node chạy MQTT trực tiếp.
- 4G modem.
- Production certification, attestation và signed OTA hoàn chỉnh.

Các capability này chỉ là roadmap, không phải feature đã hoàn thành.
