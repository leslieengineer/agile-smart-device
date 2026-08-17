# nanoESP32-C6 Board

New engineers should read the [hardware handbook](../../docs/handbook/03-phan-cung-node.md) before changing pins or safety behavior.

- Button: GPIO9, active-low with pull-up. GPIO9 is a strapping pin; holding it during reset enters download mode.
- Relay: GPIO10, active-high.
- Relay state LED: GPIO2, active-high and mirrors relay state.
- Matter status WS2812: GPIO8, RMT-driven in the `matter_node` profile.
- Flash: 16 MB.

The relay driver input requires an external fail-safe bias so the relay remains off before firmware configures GPIO10.
