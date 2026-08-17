# Hardware-in-the-Loop Acceptance

Execution/evidence guidance is documented in the [testing handbook](../../docs/handbook/09-kiem-thu-va-bang-chung.md).

HIL automation is added only with connected hardware and an explicit flash request. The current board acceptance matrix is:

1. Erased NVS boots with relay GPIO10 and LED GPIO2 OFF.
2. A short GPIO9 press toggles exactly once.
3. Twenty short presses produce twenty state changes without misses or duplicates.
4. Holding GPIO9 for three seconds produces no short-press toggle.
5. Relay ON and OFF states both survive a power cycle.
6. GPIO10 does not chatter during reset; hardware fail-safe bias is verified with a probe.
7. Holding GPIO9 during reset enters download mode as expected for the strapping pin.
8. Host logs contain no task-stack overflow, watchdog, NVS, or GPIO errors.
9. A released hold of at least five seconds opens/reopens the Matter commissioning window without toggling the relay.
10. A released hold of at least ten seconds performs Matter factory reset only after visual confirmation and does not erase the product relay-state policy.
11. GPIO8 WS2812 shows commissioning, operational, Thread transition and fault states while GPIO2 continues to mirror the relay.
12. BBB commissions a factory-new node through BLE and supplies its active Thread dataset.
13. BBB discovers the On/Off Plug-in Unit endpoint and invokes Off/On/Toggle through cluster `0x0006`.
14. A local GPIO9 short press produces an unsolicited OnOff subscription report through BBB to WebUI.
15. Node, OTBR, Matter Controller and Gateway restart tests recover inventory, CASE connectivity, subscriptions and final state.
16. Loss of BBB/Thread does not block local button-to-relay operation.
17. The five-second gesture opens both the Rhophi claim window and initial Matter window; a non-gestured node is not claimable.
18. A phone discovers and identifies exactly the selected Rhophi device.
19. BBB accepts a valid HMAC proof and rejects replay of the same nonce.
20. Five proof issuances trigger persistent lockout; power cycling does not clear it.
21. A second phone cannot read the owner connection's response, cancel it or consume its claim session.
22. PASE and attestation succeed with per-device manufacturing material; no grant or credential appears in logs.
23. Thread Operational Dataset commissioning attaches the node to OTBR without using MQTT.
24. The temporary mobile fabric opens an Enhanced Commissioning Window and BBB commissions on-network as a second fabric.
25. BBB describe/read/subscribe succeeds and a local GPIO9 press reaches Mobile/WebUI as OnOff state.
26. Temporary mobile fabric removal happens only after BBB is operational.
27. Forced removal failure leaves `CLEANUP_PENDING`; retry succeeds without losing the BBB fabric.
28. Two devices from one manufacturing batch reject each other's proof and have distinct claim IDs, secrets, discriminators and passcodes.

Verified by automated gates in the current implementation session: host tests, Matter compilation/link, dashboard build/tests/bundles and mobile TypeScript build/tests. Physical claim, native Android commissioner, relay observation, long-press UX and BBB commissioning/subscription remain pending HIL. Android HIL is additionally blocked until the pinned ConnectedHomeIP Android submodules and production attestation inputs are available.

No test in this directory may flash, erase, or reset a board unless the caller explicitly enables the hardware runner and selects the serial/debug probe.
