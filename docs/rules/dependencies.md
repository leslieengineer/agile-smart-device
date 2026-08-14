# Third-Party Dependency Policy

Use proven upstream components instead of rewriting infrastructure when they meet the requirement. Preferred foundations include ESP-IDF, FreeRTOS, lwIP, mbedTLS, and coreMQTT.

Before adding or updating a dependency:

- Define the exact use case and why standard/project facilities are insufficient.
- Pin a version, commit, lockfile entry, or ESP-IDF managed-component version.
- Review direct and transitive licenses against the product distribution model.
- Assign an owner for CVE monitoring and updates.
- Record flash, RAM, stack, task, socket, and persistent-storage impact.
- Confirm the dependency can be wrapped behind the correct Layer 3 or Layer 4 boundary.
- Add build and behavior tests, including offline/error behavior.
- Require explicit approval for GPL/AGPL or other reciprocal/copyleft additions.

Application code must not include Wi-Fi, cellular, LoRa, socket, MQTT, TLS, or cloud SDK APIs directly. Those dependencies remain behind platform adapters and reusable connectivity/protocol services.
