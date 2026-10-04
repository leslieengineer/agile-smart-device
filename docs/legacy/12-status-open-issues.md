# Trạng thái và open issues

Cập nhật: 2026-08-19 theo working tree chưa commit. Bảng này phải được cập nhật sau mỗi acceptance run.

## Trạng thái

| Hạng mục | Source | Deployed | HIL | Ghi chú |
|---|---|---|---|---|
| Firmware OnOff node | Có | ESP32-C6 | Từng phần | endpoint 1 observed |
| Claim GATT + PSA HMAC | Có | Có | Từng bước | auto window/no device lockout |
| Android native commissioner | Có | Debug APK | Từng bước | ConnectedHomeIP staged |
| Thread/IPv6 route | Có/config | BBB + Android | Route verified | unicast RIO lab workaround |
| BBB Matter Controller | Có | 0.17.9 | RPC health | bundle compatibility required |
| Provisioning BFF | Có | Enabled | Từng phần | encrypted file registry |
| ECW/BBB handoff | Có | Có | Đang debug | final clean run chưa đạt |
| Temporary fabric cleanup | Có | Có | Chưa | terminal evidence thiếu |
| Dynamic inventory | Có | Có | Chưa với node thật cuối |
| On/Off end-to-end | Có | Có | Chưa với permanent fabric |
| Restart recovery | Có một phần | Có một phần | Chưa |
| Apple Home development | Matter foundation | Chưa | Chưa | cần QR/manual, Apple Thread hub, HIL |
| Google Home development | Matter foundation | Chưa | Chưa | cần Developer Console integration, QR/manual, Google Thread hub |
| Consumer Apple/Google | Chưa | Không | Không | CSA/ecosystem certification blocker |
| Production security | Một phần | Không | Không | release blocker |

## Open issues ưu tiên

1. Hoàn tất một clean commissioning run tới BBB-only fabric.
2. Xác nhận controller `commissionOnNetwork` bundle/API tương thích deployed.
3. Xác nhận AddNOC/CommissioningComplete terminal status, không abort theo intermediate status.
4. Xác nhận RemoveFabric dùng đúng mobile fabric index.
5. Reconcile inventory metadata và online state sau restart.
6. Chuyển unicast per-phone RIO thành production LAN routing.
7. Sinh QR/manual onboarding payload và Multi-Admin share UX.
8. Google Home Developer Console test integration và Google Thread HIL.
9. Apple Home development pairing và Apple Thread Border Router HIL.
10. Production DAC/PAI/CD, CSA VID/PID/certification, secure boot, flash encryption và signed OTA.
11. Android instrumentation/HIL automation và release signing.

## Rủi ro drift

`dashboard-reference` và `mobileapp-reference` có working-tree changes ngoài submodule commit đang pin. Tài liệu này mô tả working tree được build/deploy trong phiên tích hợp; cần commit ở repo con rồi cập nhật submodule SHA trước release.

## Quy tắc đóng issue

Issue chỉ đóng khi có source change, automated regression test nếu phù hợp, deployed artifact và evidence HIL đã redact. Screenshot hoặc log stage trung gian không đủ.