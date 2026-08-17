# Checklist Bàn giao cho Nhóm ESP32-C6 Node

## 1. Repository và build

- [ ] repository URL và branch/tag
- [ ] ESP-IDF version pin
- [ ] ESP-Matter commit/submodule pin
- [ ] tool installation instructions
- [ ] clean build command
- [ ] flash command
- [ ] sdkconfig defaults committed
- [ ] partition table committed
- [ ] build ID/version visible in logs
- [ ] CI build hoặc reproducible build evidence

## 2. Hardware

- [ ] exact board/SKU name
- [ ] schematic revision
- [ ] GPIO/pin assignment
- [ ] actuator electrical limits
- [ ] sensor range/calibration
- [ ] power/brownout behavior
- [ ] antenna option/layout
- [ ] local button/reset gesture
- [ ] production debug port policy

## 3. Matter composition

- [ ] device type và revision
- [ ] endpoint 0 composition
- [ ] application endpoints
- [ ] server/client clusters
- [ ] attributes và data types
- [ ] accepted commands
- [ ] cluster revisions
- [ ] standard versus vendor-specific justification
- [ ] Descriptor dump từ controller

## 4. State behavior

- [ ] boot/default state
- [ ] local control behavior
- [ ] remote command behavior
- [ ] attribute reporting
- [ ] persistence policy
- [ ] transition/timer behavior
- [ ] offline behavior
- [ ] fault/safety behavior
- [ ] factory reset behavior

## 5. Commissioning và security

- [ ] QR/manual code cho dev units
- [ ] discriminator/passcode strategy
- [ ] VID/PID strategy
- [ ] DAC/PAI/PAA strategy
- [ ] factory reset verified
- [ ] fabric removal verified
- [ ] commissioning retry/fail-safe verified
- [ ] secure boot decision
- [ ] flash encryption decision
- [ ] signed OTA and key ownership
- [ ] no secret in logs/repository

## 6. Thread

- [ ] join target Thread network
- [ ] attach after power cycle
- [ ] attach after OTBR restart
- [ ] parent loss behavior
- [ ] IPv6 reachability
- [ ] RSSI/link test in target enclosure/location
- [ ] power consumption measurement if battery-powered

## 7. Test evidence

- [ ] unit test report
- [ ] HIL test report
- [ ] command/attribute test vectors
- [ ] local-to-remote state sync evidence
- [ ] invalid input behavior
- [ ] node offline/recovery behavior
- [ ] watchdog/reset reason evidence
- [ ] OTA success/rollback evidence
- [ ] known issues list

## 8. Gateway integration packet

Nhóm node cần gửi

- endpoint/cluster table
- sample Descriptor/Basic Information output
- operational Node ID sau commissioning test
- command examples theo Matter SDK, không phải MQTT JSON
- attribute subscription examples
- Matter status/fault mapping
- firmware logs cho một happy path và một failure path

Nhóm Gateway cần gửi lại

- discovered endpoint map
- JSON ↔ Matter mapping table
- MQTT test vectors
- timeout/retry policy
- node inventory entry
- end-to-end test report

## 9. Review questions trước merge

1. Firmware có chứa MQTT topic hoặc JSON gateway contract không? Nếu có, boundary đang sai.
2. Local change có phát Matter attribute report không?
3. Command thất bại có để attribute/hardware lệch nhau không?
4. Node mất mạng có còn an toàn và điều khiển local được không?
5. Endpoint map có dựa trên Matter device type chuẩn không?
6. Có test power cycle giữa một transition không?
7. Có credential dùng chung giữa nhiều production device không?
8. Có cách xác định chính xác firmware/board revision từ log không?

## 10. Sign-off

| Vai trò | Người duyệt | Ngày | Kết quả |
|---|---|---|---|
| Firmware lead | | | |
| Gateway lead | | | |
| Hardware lead | | | |
| Security | | | |
| Test/QA | | | |
| Product/Safety nếu áp dụng | | | |
