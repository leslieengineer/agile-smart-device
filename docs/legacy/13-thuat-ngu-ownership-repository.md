# Thuật ngữ, ownership và repository

## Trạng thái

| Hạng mục | Source | Deployed | HIL |
|---|---|---|---|
| Repository ownership | Có | Không áp dụng | Không áp dụng |
| Terminology/status vocabulary | Có | Không áp dụng | Không áp dụng |
| Submodule pins | Có | Working tree drift | Không áp dụng |

## Thuật ngữ

| Thuật ngữ | Nghĩa trong dự án |
|---|---|
| Node | ESP32-C6 application device có relay/Matter endpoint |
| RCP | ESP32-C6 radio co-processor do OTBR sở hữu |
| Claim | Xác minh per-device ownership bằng custom GATT/HMAC |
| Commissioning | Matter PASE/attestation/network/fabric setup |
| Claim window | Cửa sổ custom Rhophi |
| Matter window | Basic/Enhanced Commissioning Window |
| Temporary fabric | Fabric do mobile controller tạo |
| Permanent fabric | Fabric do BBB Matter Controller sở hữu |
| PASE | Passcode authenticated session lúc commissioning |
| CASE | Certificate authenticated operational session |
| ECW | Enhanced Commissioning Window cho fabric tiếp theo |
| OMR | Thread Off-Mesh-Routable prefix |
| RIO | IPv6 Route Information Option trong RA |
| BFF | Backend-for-Frontend giữ credential phía server |

## Repository ownership

```text
agile-smart-device/
├── components/                  firmware product/board
├── external/agile-firmware-framework/  reusable framework submodule
├── dashboard-reference/         BBB/WebUI/controller source submodule
├── mobileapp-reference/         mobile/SDK/native plugin submodule
├── reference/                   legacy reference submodule
├── tests/                       firmware architecture/host/HIL contract
├── tools/                       build/flash/manufacturing/checkers
└── docs/                        authoritative documentation
```

## Quy tắc file

- `build*`, `dist`, `node_modules`, cache và generated APK/bin không phải source.
- Component README chỉ mô tả local ownership và link lên docs.
- Tài liệu authoritative tên không dấu, kebab-case và prefix số.
- Course/full-context/reference phải có legacy banner ở entry point.

## Status vocabulary

Chỉ dùng `Có`, `Chưa`, `Đang debug`, `Không áp dụng` trong cột Source/Deployed/HIL và phải kèm evidence/lý do. Không dùng “hoàn thành” khi final HIL chưa pass.

## Citation

Fact kỹ thuật trỏ tới path và symbol/source block ổn định. Line number chỉ bổ sung khi có ích vì dễ drift. Submodule fact ghi rõ repo con.

## Ownership thay đổi

| Thay đổi | Tài liệu phải cập nhật |
|---|---|
| Endpoint/cluster/board pin | 02, 07, 10 |
| Claim wire/crypto/window | 03, 04, 09, 11 |
| Mobile state/native callback | 04, 10, 11 |
| RPC/API/MQTT/SSE | 05, 07, 10 |
| Thread prefix/routing | 06, runbook lab |
| Toolchain/partition | 08, 09 |
| HIL result | 10, 12 |

## Nguồn sự thật

`AGENTS.md`, `docs/rules/`, component CMake/source, lockfiles và test gates.