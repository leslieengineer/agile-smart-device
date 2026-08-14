# Local Gateway Before Cloud

## Target topology

```text
ESP32-C6 node
  -> transport adapter and protocol service
  -> local MQTT broker on Linux Gateway
  -> local rules, storage, alerts, and Web UI
  -> optional cloud uplink to AWS, Azure, or another backend
```

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
