# Contract MQTT và Gateway Translation

Tài liệu này dành cho Gateway/Web. Firmware node không implement JSON này.

## Topics

| Topic | Producer | Consumer | Retained |
|---|---|---|---|
| `home/control/tx` | WebUI | Gateway | không |
| `home/control/rx` | Gateway | WebUI | không |
| `home/control/status` | Gateway/LWT | WebUI/monitoring | có |

Command/response dùng QoS 1. Production cần deduplicate hoặc bảo đảm command idempotent vì QoS 1 có thể giao trùng.

## Request envelope

```json
{
  "request_id": "11111111-1111-4111-8111-111111111111",
  "node_id": "0x0000000000000001",
  "endpoint": 1,
  "cluster": "OnOff",
  "command": "On",
  "payload": {}
}
```

Quy tắc

- `request_id` là UUID correlation
- `node_id` là operational Matter Node ID, không phải MAC/IP/serial
- `endpoint` là uint16 logic
- `cluster` và `command` có thể là alias hoặc ID tại biên
- gateway normalize alias thành numeric ID
- `payload` phải là object và đúng schema command
- top-level field lạ bị từ chối
- message tối đa 8 KiB

## Success response

```json
{
  "request_id": "11111111-1111-4111-8111-111111111111",
  "node_id": "0x0000000000000001",
  "endpoint": 1,
  "cluster": 6,
  "command": 1,
  "status": "ok",
  "result": {
    "attributes": {
      "OnOff": true
    }
  },
  "latency_ms": 42.5,
  "timestamp": "2026-08-16T10:00:00.000Z"
}
```

## Error response

```json
{
  "request_id": "11111111-1111-4111-8111-111111111111",
  "node_id": "0x0000000000000001",
  "endpoint": 1,
  "cluster": 8,
  "command": 0,
  "status": "error",
  "error": {
    "code": "INVALID_PAYLOAD",
    "message": "Command payload is invalid"
  },
  "latency_ms": 2.1,
  "timestamp": "2026-08-16T10:00:00.000Z"
}
```

Error codes hiện tại

- `INVALID_ENVELOPE`
- `UNKNOWN_CLUSTER`
- `UNKNOWN_COMMAND`
- `INVALID_PAYLOAD`
- `NODE_UNKNOWN`
- `NODE_UNREACHABLE`
- `TIMEOUT`
- `PAYLOAD_TOO_LARGE`
- `CONTROLLER_ERROR`
- `INTERNAL`

Matter adapter thật cần map Interaction Model status thành code/details phù hợp, không làm mất Matter status gốc trong log.

## Attribute event

```json
{
  "type": "event",
  "request_id": null,
  "node_id": "0x0000000000000001",
  "endpoint": 1,
  "cluster": 6,
  "attributes": {
    "OnOff": true
  },
  "timestamp": "2026-08-16T10:00:00.000Z"
}
```

Event có thể đến từ local input, subscription report hoặc command khác. WebUI phải xử lý event độc lập response.

## Translation responsibilities

Gateway adapter chuyển

```text
node_id + endpoint + cluster_id + command_id + payload
```

thành Matter InvokeRequest. Nó không chuyển thành ASCII/UART frame. RCP chỉ nhận Spinel radio operations do OTBR tạo.

## Compatibility rules

- thay đổi field bắt buộc cần versioning/migration
- thêm optional field phải được hai phía chấp nhận
- alias không phải wire canonical trong response
- không tái sử dụng command ID với semantic mới
- vendor command cần vendor ID thực, schema và version rõ
- mọi thay đổi contract phải đi cùng test frontend/gateway
