# nanoESP32-C6 Board

- Button: GPIO9, active-low with pull-up. GPIO9 is a strapping pin; holding it during reset enters download mode.
- Relay: GPIO10, active-high.
- Status LED: GPIO2, active-high and mirrors relay state.
- Flash: 16 MB.

The relay driver input requires an external fail-safe bias so the relay remains off before firmware configures GPIO10.
