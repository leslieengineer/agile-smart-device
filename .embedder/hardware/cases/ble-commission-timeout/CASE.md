---
title: Android Matter BLE commissioning timeout
status: open
opened: 2026-08-19
hardware: ESP32-C6, Android A101SH, BeagleBone Black OTBR
tags: [matter, ble, thread, android]
---

## Symptom

Android enters Matter commissioning, ESP32-C6 stops BLE advertising, then mobile appears stuck and eventually times out. Retrying could also make the node disappear from scans.

## Hypotheses

| Hypothesis | Verdict | Evidence |
|---|---|---|
| PASE credential mismatch | Rejected | `onPairingComplete(errorCode=0)` and all stages through attestation succeeded. |
| BLE/GATT handoff ownership failure | Rejected | ConnectedHomeIP progressed through PASE and commissioning stages over the supplied GATT. |
| Attestation continuation called on wrong thread | Confirmed/fixed | Flow stopped at `AttestationVerification`; CHIPTool posts `continueCommissioning()` to UI thread. App now uses main `Handler`. |
| Missing Android multicast permission | Confirmed/fixed | App crashed in `WifiManager.MulticastLock.acquire()` without `CHANGE_WIFI_MULTICAST_STATE`. Permission added to plugin manifest. |
| Android lacks route to Thread OMR prefix | Confirmed/infrastructure fixed | `FindOperationalForStayActive` failed `0x02000065` (`ENETUNREACH`); Android table 1020 lacked `fd41:.../64`. OTBR sent multicast RIO but AP did not bridge it. BBB `radvd` now unicasts the RIO to the phone and Android installs the route. |
| Scan connection prevents re-advertising | Confirmed/fixed | Scan left GATT open. Scan now closes its identity session, and reset/cancel closes all sessions before rescanning. |
| Claim window unavailable after reboot/retries | Confirmed/fixed | Matter window and custom claim window were independent; valid proof generation also incremented a persistent lockout counter. Firmware now opens the claim window with an empty-fabric Matter window and relies on backend claim rate limiting instead of locking valid device proofs. |

## Findings

- ESP32 reason `0x213` is remote-user termination from Android, not the primary root cause.
- PASE and attestation are valid with lab passcode `20202021`.
- After posting attestation continuation to the Android main thread, commissioning reached `ThreadNetworkEnable` and operational discovery.
- Android Wi-Fi and BBB Ethernet are on the same IPv6 LAN and can ping directly, but multicast OTBR RIO packets are filtered between wired and Wi-Fi segments.
- A unicast RA from BBB installs `fd41:cb6b:602c:1::/64 via fe80::d239:72ff:fe31:2912` on Android.
- The Android listener must treat `onCommissioningComplete` as terminal; intermediate `SendNOC` status values are logged but no longer abort the coroutine early.
- BBB now runs the controller bundle that implements `commissionOnNetwork`; RPC health reports matter.js 0.17.9 ready.
- ESP32 NVS and Android CHIP controller preferences were cleared for a clean final commissioning attempt.
- The current BBB provisioning store contains one persisted `CLEANUP_PENDING` transaction in `/var/lib/matter-web-auth/provisioning-transactions.json`; it is not expired and blocks a new session for the same claim.
- The BFF CORS preflight currently allows only `GET, POST, OPTIONS`, while Android cancellation uses `DELETE`; Android logcat confirms the DELETE preflight is rejected, so the cleanup transaction cannot be canceled through the app.
- The BLE UUID contract is not reversed: Android uses canonical `9a7d5210-...` UUIDs and firmware uses the required NimBLE little-endian storage order.

## Next probe
Allow DELETE in BFF mobile CORS, add a DELETE preflight regression test, redeploy BFF, then cancel the persisted cleanup transaction and retry one clean commissioning attempt. Verify `CommissioningComplete`, BBB on-network handoff, temporary fabric removal, and `/api/devices` inventory.

