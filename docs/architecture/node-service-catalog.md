# Node Service Catalog

> **LEGACY / NON-AUTHORITATIVE.** Dùng [firmware authoritative](../02-firmware-esp32c6.md).

Implemented framework services: binary switch, configuration, security policy, network manager, provisioning, indication, messaging ports, offline queue, telemetry, command dispatcher, time sync, diagnostics, health monitor, and OTA policy.

Products compose only required services. Wi-Fi, 4G, coreMQTT/TLS, portal, time-source, indicator, watchdog/recovery, firmware-store, boot-control, digest, and signature implementations remain product/platform adapters.

Deferred: alerting is Gateway-owned; calibration/sampling/filtering/data pipeline require a sensor product; data logging is replaced by node offline queue plus Gateway history; scheduler remains product runtime; power management requires a battery/sleep product; BLE Mesh/scenes are not scheduled.
