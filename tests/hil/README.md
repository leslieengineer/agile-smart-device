# Hardware-in-the-Loop Acceptance

HIL automation is added only with connected hardware and an explicit flash request. The current board acceptance matrix is:

1. Erased NVS boots with relay GPIO10 and LED GPIO2 OFF.
2. A short GPIO9 press toggles exactly once.
3. Twenty short presses produce twenty state changes without misses or duplicates.
4. Holding GPIO9 for three seconds produces no short-press toggle.
5. Relay ON and OFF states both survive a power cycle.
6. GPIO10 does not chatter during reset; hardware fail-safe bias is verified with a probe.
7. Holding GPIO9 during reset enters download mode as expected for the strapping pin.
8. Host logs contain no task-stack overflow, watchdog, NVS, or GPIO errors.

No test in this directory may flash, erase, or reset a board unless the caller explicitly enables the hardware runner and selects the serial/debug probe.
