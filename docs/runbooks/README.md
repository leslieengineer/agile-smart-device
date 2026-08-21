# Runbook vận hành

Runbook chứa thao tác thay đổi trạng thái. Tài liệu kiến trúc chỉ chứa kiểm chứng read-only.

- [Bench lab](lab-hardware.md)
- [Firmware build và flash](firmware-build-flash.md)
- [Android build và install](android-build-install.md)
- [BBB deploy và rollback](bbb-deploy-rollback.md)
- [Commissioning end-to-end](commissioning-e2e.md)
- [Recovery và reset](recovery-reset.md)

## Quy tắc

1. Xác minh target/port/host trước thao tác.
2. Backup state cần giữ.
3. Không đưa password, token, dataset, key hoặc grant vào command history/evidence.
4. Dừng nếu expected health gate không đạt.
5. Ghi rollback trước khi cutover.