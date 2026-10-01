# Telemetry Service

Bounded boolean telemetry encoder and delivery policy. It publishes through `IMessageTransport`, queues while offline, and never emits truncated JSON. Gateway arrival time is used when node UTC is unavailable.
