# Local Gateway Before Cloud

> **LEGACY / NON-AUTHORITATIVE.** Dùng [BBB architecture](../05-bbb-gateway-controller-webui.md).

## System topology

Bản đồ runtime, process, protocol, port và data flow hiện tại nằm tại [kiến trúc toàn hệ thống](system-overview.md).

Cloudflare Tunnel hiện cung cấp remote ingress tới local BBB Gateway. Đây không phải cloud telemetry backend; uplink bất đồng bộ tới AWS, Azure hoặc backend khác vẫn là planned capability.

## Required behavior

- Local button/relay control remains functional without Wi-Fi, Gateway, Internet, or Cloud.
- The Linux Gateway owns edge aggregation, filtering, local history, alert evaluation, and local UI integration.
- Cloud publication is asynchronous and optional; Gateway reconnect/backoff cannot block local rules.
- Node application code publishes semantic data through an application-facing contract, never through Wi-Fi, sockets, MQTT, or cloud SDK APIs.
- Connectivity lifecycle/reconnect belongs in reusable Layer 4 services; radio/socket/TLS implementation belongs in Layer 1/3 adapters and protocol components.
- Payload schema, subject/topic rules, delivery semantics, and offline queue requirements must be defined before adding a publisher interface.

## Contract promotion trigger

Do not add an `IMessagePublisher` yet. Introduce it with the first concrete telemetry or command use case, after defining:

1. bounded payload representation,
2. subject/topic ownership,
3. delivery and retry semantics,
4. offline behavior,
5. authentication and provisioning boundary,
6. host tests using a fake publisher.

This avoids coupling the application to a guessed MQTT API while preserving a clean path from Local Gateway to optional Cloud.
