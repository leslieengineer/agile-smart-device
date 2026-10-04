# 03 — Phần cứng ESP32-C6 application node

## Board profile

Board hiện được mô tả là nanoESP32-C6, ESP32-C6 revision v0.2, flash vật lý 16 MiB DIO. Matter dùng native IEEE 802.15.4 radio; BLE phục vụ commissioning.

## Pinout

| Chức năng | GPIO | Direction | Polarity | Ghi chú |
|---|---:|---|---|---|
| Button | 9 | Input | Active-low | Internal pull-up, both-edge interrupt, strapping pin. |
| Relay | 10 | Output | Active-high | Cần external fail-safe OFF bias. |
| Relay LED | 2 | Output | Active-high | Mirror relay state. |
| Matter WS2812 | 8 | Output/RMT | GRB | Commissioning/Thread/fault indication. |

Source authoritative là `components/board_esp32c6/include/board/BoardPins.hpp`.

## GPIO9 strapping risk

GPIO9 là strapping/download pin. Nếu giữ nút trong lúc reset, ESP32-C6 có thể vào download mode thay vì boot application. Factory-reset UX phải yêu cầu nhấn giữ sau khi firmware đã boot và phải được HIL kiểm chứng để tránh nhầm với download gesture.

## Relay safe state

Yêu cầu phần cứng:

1. relay driver phải có bias giữ OFF khi MCU reset/high impedance;
2. không phụ thuộc firmware để đảm bảo trạng thái trước boot;
3. GPIO10 không được chatter trong reset/brownout;
4. kiểm chứng bằng probe/oscilloscope, không chỉ nhìn log.

Firmware cấu hình output safe-low trước khi restore state, nhưng điều này không thay thế fail-safe bias vật lý.

## Power restore policy

Policy đã chọn là restore trạng thái hợp lệ từ NVS. Nếu schema/key thiếu hoặc lỗi, state mặc định OFF. NVS write chậm 500 ms để giảm wear, vì vậy mất điện ngay sau command có thể restore state trước đó; behavior này phải được ghi trong product requirement/HIL.

## Status indication

GPIO2 chỉ phản ánh relay. GPIO8 dành cho lifecycle:

| Màu | Ý nghĩa hiện tại |
|---|---|
| Amber | Commissioning window/fabric removed. |
| Green | Commissioning complete. |
| Blue | Thread interface transition. |
| Red | Commissioning fail-safe timeout. |
| White | Identify. |

Pattern/visual confirmation cuối cùng chưa HIL-approved.

## Điểm phần cứng còn thiếu evidence

- Schematic và PCB revision được version hóa.
- Antenna layout/orientation và enclosure impact.
- Power supply limits và relay coil/current budget.
- Brownout threshold/behavior.
- Relay contact rating, isolation/creepage và load class.
- EMI/ESD/surge evidence.
- WS2812 part/order/brightness/current.

## Bring-up checklist

- [ ] Xác nhận chip là ESP32-C6 revision mong đợi.
- [ ] Xác nhận flash 16 MiB.
- [ ] Đo GPIO10 trong reset/power ramp.
- [ ] Xác nhận button released = high, pressed = low.
- [ ] Xác nhận GPIO2 mirror relay.
- [ ] Xác nhận WS2812 GPIO8 đúng GRB/RMT.
- [ ] Test short press một lần tạo đúng một toggle.
- [ ] Test 20 presses không miss/duplicate.
- [ ] Test power restore ON/OFF.
- [ ] Test GPIO9 download mode có chủ đích.
- [ ] Lưu board revision, firmware SHA và measurement evidence.

Xem thêm [board README](../../components/board_esp32c6/README.md) và [HIL matrix](../../tests/hil/README.md).
