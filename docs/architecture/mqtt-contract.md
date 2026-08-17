# MQTT contract catalog note

## Trạng thái

Namespace `asd/v1/<site>/<device>` dưới đây là **framework/alternative Gateway-connected profile contract**. Nó không phải contract đang deploy của sản phẩm Matter-over-Thread hiện tại và không được compile vào `matter_node`.

Active deployed WebUI/BBB contract dùng:

```text
home/control/tx
home/control/rx
home/control/status
```

Source-of-truth active xem [`docs/full-context/03-contract-mqtt-gateway.md`](../full-context/03-contract-mqtt-gateway.md) và sibling `agile-dashboard/packages/contracts/src`.

## Alternative framework namespace

Base namespace: `asd/v1/<site>/<device>`.

Các topic dự kiến gồm retained QoS1 status/state, QoS0 telemetry/diagnostics, QoS1 command request/response và OTA request/event. Wi-Fi keepalive dự kiến 60 giây; metered 4G 120 giây. Identity change yêu cầu clean session.

Framework policy dùng bounded topics/payload, offline queue và idempotency window. MQTT wire handling dự kiến qua coreMQTT `IMessageTransport`.

## Boundary bắt buộc

Application node hiện tại không include MQTT, socket, Wi-Fi, cellular, TLS hoặc cloud API. Mọi MQTT/JSON nằm ở WebUI/BBB Gateway boundary. Không dùng tài liệu alternative này để phát message vào production broker.
