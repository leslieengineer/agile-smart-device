# 06 — BBB Gateway và WebUI

Nội dung canonical nằm trong sibling repository `C:\Users\lesli\WS\agile-dashboard`. Chương này chỉ mô tả boundary cần thiết cho node team.

## As-built stack

```text
Browser
  -> Vue WebUI
  -> MQTT over WebSocket :9001
  -> Mosquitto
  -> Node.js Gateway
  -> Unix JSON-lines RPC
  -> matter.js Controller 0.17.9
  -> Matter/CASE/IPv6
  -> OTBR wpan0
  -> Thread node
```

## WebUI

- Vue 3, TypeScript, Vite, Pinia và TailwindCSS.
- Static files đang được `matter-webui.service` serve trên port 8080.
- Browser nhập MQTT credential lúc runtime; không cần bundle password.
- Dashboard gửi typed command và consume correlated response/unsolicited event.

Source: `agile-dashboard/apps/webui/src`.

## MQTT broker

Listeners hiện được thiết kế:

- TCP 1883 loopback cho Gateway.
- WebSocket 9001 cho browser.
- Anonymous disabled.
- ACL tách user Gateway và WebUI.

Không đưa broker password vào tài liệu/Git.

## Active MQTT contract

| Topic | Direction | QoS/retained |
|---|---|---|
| `home/control/tx` | WebUI → Gateway | QoS 1 |
| `home/control/rx` | Gateway → WebUI | QoS 1 |
| `home/control/status` | Gateway/LWT → clients | Retained |

Request fields:

```json
{
  "request_id": "uuid",
  "node_id": "0x0000000000000001",
  "endpoint": 1,
  "cluster": 6,
  "command": 1,
  "payload": {}
}
```

Gateway giới hạn envelope 8 KiB, reject unknown fields/IDs và map typed error. QoS 1 đòi hỏi idempotency/deduplication ở boundary phù hợp.

Xem [active contract](../full-context/03-contract-mqtt-gateway.md).

## Gateway

Gateway sở hữu:

- MQTT connection/LWT;
- Zod validation;
- symbolic-to-numeric normalization;
- cluster command registry;
- timeout/error mapping;
- response/event publication;
- Controller abstraction.

Gateway không sở hữu relay state và không mở RCP serial.

### Current mode

Gateway production service đang chạy, nhưng control path vẫn ở mock mode khi chưa có commissioned application node. Không dùng mock success làm Matter acceptance evidence.

## Matter Controller service

- matter.js 0.17.9.
- Long-lived systemd service.
- Storage `/var/lib/matter-controller`.
- Socket `/run/matter-controller/controller.sock`.
- Group permission `matter-rpc`, mode 0660.
- Gateway là client; browser không truy cập socket.

RPC hiện có:

```text
health
listNodes
invoke
```

Còn thiếu trước HIL:

```text
commission
removeNode/decommission
explicit read
subscribe/unsubscribe
attribute/event streaming
inventory/subscription recovery verification
```

Source: `agile-dashboard/packages/matter-controller/src`.

## OTBR và RCP

- BBB chạy `otbr-agent`.
- RCP là ESP32-C6 riêng qua USB Spinel/HDLC 460800.
- Interface Thread là `wpan0`.
- Audit xác nhận `otbr-agent` active và role leader.
- Child table rỗng khi audit vì application node chưa commission.

RCP chỉ cung cấp radio. Matter Controller thực hiện application commissioning.

## systemd và security boundaries

Service users tách biệt. Các unit dùng hardening như `NoNewPrivileges`, `ProtectSystem`, `ProtectHome`, `PrivateDevices` và memory limits tùy service. Controller storage không được Gateway/WebUI đọc trực tiếp.

## Persistent state

- Fabric/controller → `/var/lib/matter-controller`
- Broker → `/var/lib/mosquitto`
- Thread → `/var/lib/thread`
- Runtime socket → `/run/matter-controller`

Backup storage trước upgrade. Xóa Controller storage làm mất fabric và yêu cầu recommission.

## Canonical sibling references

```text
agile-dashboard/docs/full-context/10-linux-gateway-as-built.md
agile-dashboard/docs/full-context/03-contract-mqtt-gateway.md
agile-dashboard/docs/full-context/09-matter-controller-service.md
agile-dashboard/docs/full-context/11-production-web-auth.md
agile-dashboard/deploy/README.md
agile-dashboard/packages/contracts/src/
agile-dashboard/packages/gateway/src/
agile-dashboard/packages/matter-controller/src/
agile-dashboard/apps/webui/src/
```
