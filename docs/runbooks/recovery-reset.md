# Recovery và reset

## Chọn phạm vi nhỏ nhất

| Vấn đề | Recovery |
|---|---|
| GATT cache/session | app cancel/reset, cache refresh/reconnect |
| Early BFF transaction | DELETE transaction hoặc restart BFF khi chưa persist secret |
| Temporary fabric cleanup | retry bằng cùng Android controller storage |
| Controller bundle mismatch | deploy bundle, retry handoff |
| Android route mất | cập nhật unicast RIO client/production RA |
| Orphan mobile fabric | dùng controller cũ RemoveFabric hoặc node factory reset |
| Node Matter state corrupt | factory reset/erase default NVS có kiểm soát |
| BBB controller state corrupt | restore backup, không xóa tự phát |

## Android storage

Xóa CHIP controller prefs làm mất khả năng xóa fabric cũ. Chỉ thực hiện sau khi node đã factory reset hoặc fabric được remove. Secure login token nằm store khác và không cần xóa trong recovery Matter.

## Node reset

Physical factory-reset gesture hoặc Matter factory-reset command là đường ưu tiên. Erase default NVS là thao tác phá hủy cuối cùng. Không erase partition `fctry`.

## BBB reset

Restart service không xóa state. Trước thay `/var/lib/matter-controller`, backup owner/mode và xác nhận tác động tới mọi node commissioned.

## Sau reset

1. RPC health ready.
2. Node boot có claim window khi fabric count 0.
3. Android OMR route tồn tại.
4. BFF transaction store không chứa stale transaction.
5. Chạy commissioning từ đầu và thu evidence mới.