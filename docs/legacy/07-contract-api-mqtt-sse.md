# REST, MQTT và SSE contracts

## Trạng thái

| Contract | Source | Deployed | HIL |
|---|---|---|---|
| MQTT command/response | Có | Có | Integration test |
| REST auth/command | Có | Có | Unit/integration |
| Provisioning API | Có | Có | Đang debug E2E |
| SSE realtime | Có | Có | Từng phần |

## MQTT

| Topic | Direction | Nội dung |
|---|---|---|
| `home/control/tx` | BFF/UI → Gateway | command envelope |
| `home/control/rx` | Gateway → BFF/UI | response/event |
| `home/control/status` | Gateway → clients | retained health |

Envelope chứa request UUID, Node ID dạng text 64-bit, endpoint, cluster, command và bounded payload. Gateway validate schema trước RPC.

## REST

| Route | Mục đích |
|---|---|
| `POST /api/login` | web cookie session |
| `POST /api/mobile/login` | mobile bearer token |
| `GET /api/session` | session info |
| `POST /api/logout` | revoke session |
| `GET /api/health` | BFF/MQTT health |
| `POST /api/command` | Matter command qua Gateway |
| `GET /api/events` | SSE |
| `GET /api/devices` | dynamic inventory |
| `GET/DELETE /api/devices/:node` | describe/remove |
| `/api/commissioning/sessions...` | claim/handoff/complete transaction |

Web mutating request cần origin + CSRF. Mobile mutating request cần bearer + allowed origin.

## Commissioning states

State machine gồm claim, grant, PASE, attestation, Thread, temporary fabric, ECW, BBB fabric, discovery/subscription, cleanup và terminal states. `CLEANUP_PENDING` nghĩa BBB fabric có thể đã hoạt động nhưng mobile fabric chưa xóa xong.

## SSE

BFF fan-out response/event/provisioning và ping keepalive. Mobile dùng fetch streaming để gửi Authorization. `Last-Event-ID` hỗ trợ reconnect nhưng server chưa cung cấp durable replay; client phải reconcile inventory bằng REST.

## Inventory

Controller descriptor là nguồn capability. Client không hard-code node/endpoint. Node hiện tại dự kiến OnOff endpoint 1.

## Error mapping

Domain errors giữ code ổn định; HTTP status chỉ là transport. Ví dụ transaction conflict 409, expired 410, rate limit 429, invalid claim 422 và controller failure 502.

## Nguồn sự thật

- `dashboard-reference/packages/contracts/src/`
- `dashboard-reference/packages/webui-bff/src/provisioningRoutes.ts`
- `mobileapp-reference/packages/client-sdk/src/commissioning/`