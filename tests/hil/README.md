# Hardware-in-the-Loop Acceptance

Hướng dẫn evidence nằm tại [Testing/HIL authoritative](../../docs/10-testing-hil-evidence.md) và [commissioning runbook](../../docs/runbooks/commissioning-e2e.md).

Không test nào được flash, erase hoặc reset hardware nếu caller chưa chọn đúng target và cho phép thao tác.

## Node local acceptance

- [ ] Erased default NVS boot với relay GPIO10 và LED GPIO2 OFF.
- [ ] Short press GPIO9 toggle đúng một lần; 20 lần không miss/duplicate.
- [ ] Commissioning hold theo `ButtonInputConfig` không tạo short-press toggle.
- [ ] Factory-reset hold chỉ chạy sau ngưỡng và không xóa factory partition.
- [ ] Relay ON/OFF survive power cycle.
- [ ] GPIO10 không chatter lúc reset; hardware fail-safe bias được đo.
- [ ] WS2812 thể hiện commissioning/Thread/fault; GPIO2 vẫn mirror relay.
- [ ] Mất BBB/Thread không chặn local button-to-relay.

## Claim acceptance

- [ ] Matter window và Rhophi claim window mở khi fabric count bằng 0.
- [ ] Phone scan/identify đúng node; GATT cache/reconnect không làm node biến mất.
- [ ] Identity 36 byte và product/claim ID đúng registry.
- [ ] Valid HMAC proof được BFF chấp nhận; replay nonce bị từ chối.
- [ ] Backend rate-limit invalid proof; valid proof issuance không gây device lockout.
- [ ] Connection khác không đọc/cancel claim session đang được bind.
- [ ] Không claim secret, proof, grant hoặc dataset xuất hiện trong log/evidence.

## Matter commissioning acceptance

- [ ] Android thực hiện BLE PASE và attestation bằng material lab/production phù hợp.
- [ ] Android cấp active Thread dataset và node attach OTBR mà không qua MQTT.
- [ ] Android có IPv6 OMR route trước operational discovery.
- [ ] Temporary mobile fabric mở ECW.
- [ ] BBB `commissionOnNetwork` tạo permanent fabric qua Thread/IP.
- [ ] BBB `describeNode`, OnOff read và subscribe thành công.
- [ ] Mobile xóa temporary fabric chỉ sau khi BBB operational.
- [ ] Forced cleanup failure vào `CLEANUP_PENDING`; retry không mất BBB fabric.
- [ ] Fabric table cuối chỉ còn BBB fabric.

## UI và recovery acceptance

- [ ] `/api/devices` hiển thị node/endpoint OnOff động.
- [ ] WebUI/mobile Off/On/Toggle đổi relay.
- [ ] Local short press tạo subscription report tới WebUI/mobile.
- [ ] Restart node, OTBR, controller, gateway và BFF phục hồi inventory/CASE/subscription.
- [ ] Remove và recommission thành công.
- [ ] Hai thiết bị manufacturing batch có material riêng và không chấp nhận proof chéo.

## Trạng thái 2026-08-19

Automated host/dashboard/mobile build và tests đã pass trong phiên tích hợp. Claim, PASE, attestation, Thread attach, IPv6 route và nhiều stage commissioning đã được quan sát trên hardware. ECW → BBB permanent fabric → temporary fabric cleanup → inventory/OnOff/restart vẫn phải hoàn tất trong một clean acceptance run trước khi đánh HIL pass.