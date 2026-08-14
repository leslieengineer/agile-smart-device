# MQTT Gateway Contract

Base namespace: `asd/v1/<site>/<device>`.

Topics: retained QoS1 status and state, QoS0 telemetry/diagnostics, QoS1 command request/response and OTA request/event. Wi-Fi keepalive is 60 seconds; metered 4G keepalive is 120 seconds. Identity changes request a clean session.

The node uses bounded topics/payloads and an offline queue. Command IDs are idempotent within an eight-entry window. MQTT wire handling is delegated to coreMQTT through `IMessageTransport`; application code never includes MQTT, socket, Wi-Fi, cellular, TLS, or cloud APIs.
