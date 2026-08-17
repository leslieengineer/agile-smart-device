# 11 — Vận hành và xử lý sự cố

## Nguyên tắc

1. Xác định layer lỗi trước khi thay code.
2. Thu log từ hop gần lỗi nhất.
3. Không xóa NVS/fabric/dataset như bước đầu tiên.
4. Ghi version/SHA và expected/actual.
5. Không log secret.

## Node không boot

Kiểm tra:

- đúng COM/baud;
- GPIO9 có bị giữ low;
- ROM log báo download mode hay application;
- partition table/profile có đúng;
- bootloader/app offsets từ build output;
- chip/flash revision;
- power/brownout.

Nếu ROM chờ download, release GPIO9 rồi reset đúng sequence.

## Flash không connect

- Đóng serial monitor/process giữ COM.
- Kiểm tra Device Manager/USB cable.
- Xác nhận COM bằng `chip_id`.
- Thử manual download mode có chủ đích.
- Không thử erase ngẫu nhiên trên thiết bị production.

## Relay không đúng

- Kiểm tra GPIO10 polarity và external driver.
- Kiểm tra GPIO2 chỉ là indicator, không phải relay feedback.
- Đọc NVS restore logs.
- Đợi >500 ms trước power-cycle persistence test.
- Đo physical GPIO/driver/load, không chỉ nhìn Matter attribute.

## Button false/multiple events

- Kiểm tra pull-up và wiring GPIO9.
- Kiểm tra bounce/noise/ground.
- So sánh raw edge với semantic event.
- Kiểm tra queue pressure.
- Nhớ GPIO9 là strapping pin.

ISR queue-full hiện chưa có drop counter; nếu nghi ngờ phải instrument trước khi kết luận.

## Matter endpoint không tạo

Tìm log `On/Off Plug-in Unit created`. Kiểm tra:

- đúng `matter_node` profile;
- ESP-Matter path/SHAs;
- NVS init;
- component dependencies;
- heap/resource errors.

## BLE không thấy node

- Node factory-new/window open.
- CHIPoBLE advertising log.
- BLE adapter trên Commissioner đã enable.
- Không có thiết bị khác cùng discriminator/passcode gây ambiguity.
- Window có timeout/đã đóng.

BBB Controller hiện thiếu production commission API/BLE integration, đây có thể là expected blocker chứ không phải node bug.

## Thread không attach

BBB:

- `otbr-agent` active;
- `wpan0` up;
- `ot-ctl state` leader/router;
- IPv6 route có Thread prefixes;
- RCP serial chỉ OTBR mở.

Node:

- đã nhận dataset qua commissioning;
- OpenThread started;
- radio/antenna/power khỏe;
- dataset/network match.

Không in dataset/key khi debug.

## Controller không thấy node

- Node đã commission vào đúng Controller fabric hay commissioner tạm thời khác.
- Controller storage đúng path/permission.
- Node ID trong inventory đúng 64-bit.
- mDNS/IPv6 reachability.
- CASE session/reconnect logs.
- `listNodes` và OTBR child table.

Commission bằng commissioner ngoài không làm node xuất hiện trong BBB fabric.

## Gateway timeout

- Mosquitto/Gateway/Controller active.
- Gateway mode mock hay matterjs.
- Unix socket tồn tại/permission group đúng.
- Node commissioned/connected.
- endpoint/cluster/command canonical.
- Controller invoke/read error.

## WebUI disconnected

- WebUI static service/nginx.
- MQTT WSS port 9001.
- Runtime URL/user/password.
- Mosquitto ACL.
- Retained Gateway status/LWT.
- Browser DevTools WebSocket frames.

Không hard-code credential vào frontend để chữa lỗi.

## Local event không tới WebUI

Chuỗi kiểm tra:

1. Relay/Matter local attribute log thay đổi.
2. Controller subscription alive.
3. Controller RPC event stream.
4. Gateway chuyển event đúng envelope.
5. MQTT publish `home/control/rx`.
6. WebUI validation/store nhận event.

Hiện bước 2–4 chưa implemented hoàn chỉnh.

## NVS lỗi

- Phân biệt missing state với partition corruption.
- Không erase toàn partition khi chưa biết dữ liệu khác.
- Current startup có thể erase default NVS khi no-free-pages/new-version; production policy cần review.
- Lưu log/error code/schema.

## Heap/watchdog/reboot

Thu:

- reset reason;
- free/min heap;
- task watermark;
- queue counters;
- coredump/ELF SHA;
- uptime và Thread/Controller state.

Dùng đúng ELF để decode. Không che reboot loop bằng unconditional reset/retry.

## Escalation package

Khi mở issue, gửi:

- product/toolchain SHAs;
- board/chip revision;
- sanitized logs từ node/Controller/Gateway/OTBR;
- reproduction steps;
- expected/actual;
- endpoint/cluster/path;
- physical observation;
- đã loại trừ layer nào.
