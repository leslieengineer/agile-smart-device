# Tiêu chí Kiểm thử Tích hợp

## Mục tiêu

Chứng minh từng boundary hoạt động riêng trước khi test toàn chuỗi. Không dùng “UI đổi màu” làm bằng chứng duy nhất rằng actuator/node đúng.

## Level 1 — Firmware unit/component

Nhóm node cần test

- state transition của từng command
- payload/range ở layer firmware
- local input và Matter command đi cùng state path
- persistence/debounce
- motor/relay driver fake
- safety interlock
- factory reset state machine
- OTA rollback logic

## Level 2 — Hardware-in-loop node

### Smart switch/light

- On/Off/Toggle đổi output vật lý đúng
- CurrentLevel và output PWM nhất quán
- transition bị Stop hủy đúng
- local button tạo attribute report
- reboot phục hồi theo policy đã định

### Window covering

- calibration và limit switch
- Up/Down/Stop priority
- GoTo percentage trong tolerance
- obstruction/fault không tiếp tục drive motor
- position report theo chuyển động
- power loss không chạy ngoài limit

### Cooktop prototype

- panel lock chặn command theo policy
- StopAll có ưu tiên cao
- invalid zone/power bị từ chối
- over-temperature/brownout đưa output về safe state
- network loss không vô hiệu local emergency stop

## Level 3 — Thread

- node join network qua commissioning
- attach lại sau RCP/OTBR restart
- attach lại sau power cycle node
- IPv6 reachability
- RSSI/link quality trong vị trí dự kiến
- behavior khi partition hoặc parent mất
- no excessive reconnect/power consumption

## Level 4 — Matter

- commissioning bằng QR/manual code
- descriptor/endpoint discovery đúng profile
- required clusters/attributes có mặt
- invoke command trả status đúng
- read attribute trả type/range đúng
- subscription nhận local và remote changes
- fabric removal/factory reset đúng
- multi-admin nếu trong scope

## Level 5 — Gateway adapter

Khi Matter Controller thật được implement

- node inventory ánh xạ Node ID đúng
- endpoint/cluster discovery đúng
- invoke payload map đúng SDK type
- Matter status được giữ trong logs/details
- timeout và unreachable phân biệt được
- subscription tự khôi phục sau reconnect
- duplicate MQTT request không gây tác dụng nguy hiểm

## Level 6 — End-to-end Web

| Scenario | Expected |
|---|---|
| Web bật switch | actuator bật, attribute report true, UI true |
| local switch tắt | attribute report false, UI tự cập nhật |
| level 255 | gateway trả INVALID_PAYLOAD, node không nhận invoke |
| node offline | NODE_UNREACHABLE hoặc TIMEOUT có ngữ cảnh |
| rèm Stop | motor dừng trong giới hạn thời gian safety |
| panel locked | command power bị từ chối, UI hiển thị lỗi |
| gateway restart | subscriptions được khôi phục, state resync |
| broker reconnect | request mới hoạt động, command cũ không replay |

## Test evidence cần lưu

- firmware commit và build ID
- sdkconfig/partition table
- board revision
- Thread dataset identifier không chứa secret
- Matter node/endpoint descriptor dump
- logs gateway và node có timestamp
- MQTT request/response correlation
- ảnh/video hoặc measurement cho actuator
- pass/fail và issue link

## Acceptance gate đầu tiên

Một Smart Switch node được xem là tích hợp tối thiểu khi

1. commission thành công vào Thread/Matter fabric
2. Gateway Controller discover đúng endpoint OnOff
3. Web command On/Off điều khiển output vật lý
4. local button cập nhật UI qua subscription
5. power cycle không phá fabric hoặc safe state
6. test invalid payload và node offline cho lỗi đúng
7. không có MQTT/JSON code trong node firmware
