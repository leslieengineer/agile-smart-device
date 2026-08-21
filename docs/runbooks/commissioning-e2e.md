# Commissioning end-to-end

## Clean checkpoint

- Node không có fabric, firmware/factory registry khớp.
- Android controller/transaction không stale.
- BBB RPC health ready và hỗ trợ `commissionOnNetwork`.
- OTBR leader/router và Android có OMR route.
- BFF provisioning enabled.

## Sequence

1. Mở Add Device và scan.
2. Chọn node sau khi identity được đọc mới.
3. App claim node qua BFF.
4. Xác nhận log PASE, attestation và Thread stages.
5. Xác nhận node xuất hiện trong `ot-ctl child/neighbor table`.
6. App mở ECW và BFF gọi BBB `commissionOnNetwork`.
7. Controller describe/read/subscribe node thành công.
8. Mobile xóa temporary fabric.
9. BFF complete transaction và `/api/devices` có node.
10. Test On/Off từ WebUI/mobile và physical button report ngược.

## Evidence pass

```text
Mobile: onCommissioningComplete errorCode=0
BBB RPC: commissioned_nodes chứa node
API: /api/devices trả endpoint OnOff
Node: commissioning complete/fabric lifecycle log
UI: command và local report cùng hội tụ
```

Không paste grant/dataset/token.

## Failure handling

- Trước temporary fabric: cancel và bắt đầu lại.
- Sau temporary fabric, trước BBB fabric: giữ mobile controller storage để retry ECW/handoff.
- Sau BBB fabric, cleanup lỗi: giữ `CLEANUP_PENDING`, không tạo fabric mới.
- Unknown RPC: deploy đúng controller bundle; không recommission BLE.
- IPv6 unreachable: sửa OMR route trước retry.

## Acceptance cuối

Đọc FabricList từ permanent controller và yêu cầu chỉ còn BBB fabric; reboot node/controller rồi retest On/Off.