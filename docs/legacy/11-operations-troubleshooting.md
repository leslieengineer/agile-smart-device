# Vận hành và troubleshooting

## Trạng thái

| Hạng mục | Source | Deployed | HIL |
|---|---|---|---|
| Layered diagnostics | Có | Dùng trong lab | Từng lỗi đã xác minh |
| Automated recovery | Một phần | Một phần | Chưa acceptance |
| Production observability | Chưa | Không | Không |

## Quy trình

1. Xác định tầng đầu tiên thất bại.
2. Thu log nhỏ nhất cần thiết.
3. Không reset/erase khi chưa backup state cần giữ.
4. Sửa một giả thuyết mỗi lần và chạy lại từ clean checkpoint.

## Bảng chẩn đoán

| Triệu chứng | Kiểm tra | Nguyên nhân thường gặp | Hành động |
|---|---|---|---|
| Không scan thấy node | ESP BLE log, Android permission | window đóng, GATT còn connected | mở window, cancel/close rồi scan |
| Service có nhưng thiếu characteristic | `RhophiGatt` dump | Android GATT cache | refresh cache + reconnect |
| `INVALID_DEVICE` | identity flags | claim window chưa active/cache cũ | đọc identity mới |
| `GRANT_ISSUED -> GRANT_ISSUED` | store + SSE | concurrent state owner | orchestration guard/idempotent advance |
| HTTP 409 | BFF transaction | stale early claim | cancel/replace transaction sớm |
| Treo attestation | log stage | `continueCommissioning` sai thread | post lên Android main thread |
| App crash multicast lock | AndroidRuntime | thiếu permission | thêm multicast permission |
| `FindOperational... ENETUNREACH` | Android table 1020 | thiếu OMR route | sửa RIO/radvd |
| Unknown RPC `commissionOnNetwork` | bundle/RPC health | controller bundle cũ | deploy bundle tương thích |
| `SendNOC` stage status | terminal callback + ESP OpCred log | abort quá sớm/stale fabric | chờ `onCommissioningComplete`, clean state |
| Gateway restart loop | journal/socket | controller chưa tạo socket | đợi RPC ready rồi restart gateway |

## Log filters

Android:

```bash
adb logcat -v threadtime '*:S' RhophiCommissioning:V RhophiGatt:V BluetoothGatt:V AndroidRuntime:E Capacitor:V
```

BBB:

```bash
journalctl -u otbr-agent -u matter-controller -u matter-gateway -u matter-web-auth --since '10 minutes ago' --no-pager
```

Firmware dùng serial 115200 và lọc `matter-node`, `switch-runtime`, `chip`, `OPENTHREAD`.

## Recovery boundaries

- Cancel transaction không xóa fabric đã tạo.
- Android GATT cache refresh không xóa Matter storage.
- Xóa Android CHIP prefs tạo commissioner mới và có thể để orphan fabric trên node.
- Erase node NVS xóa Matter fabric nhưng không được xóa `fctry`.
- Xóa BBB controller storage mất permanent controller identity.

Chi tiết thao tác ở [Recovery/reset runbook](runbooks/recovery-reset.md).