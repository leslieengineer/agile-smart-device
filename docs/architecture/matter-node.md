# ESP32-C6 Matter-over-Thread node

> **LEGACY / NON-AUTHORITATIVE.** Dùng [firmware authoritative](../02-firmware-esp32c6.md) và [claim GATT](../03-rhophi-claim-gatt.md). Không dùng file này làm căn cứ acceptance.

## Scope

The production node path is a Matter On/Off Plug-in Unit over Thread. MQTT and the WebUI JSON envelope terminate on the BBB Gateway and are not implemented in node firmware.

```text
WebUI -> Mosquitto -> BBB Gateway -> matter.js Controller -> OTBR -> Thread -> ESP32-C6
```

## Pinned build baseline

| Dependency | Revision |
|---|---|
| ESP-IDF | v5.5.5, `b774170ff46c393eeb5e495ea37936038d3f4f4f` |
| ESP-Matter | release/v1.6, `c91ddfbb08ccc74bb73dd6eca7422178f48b75e1` |
| ConnectedHomeIP | `93abd8e6891bb578ea63254fb29d099936f345c8` |
| Matter specification line | 1.6 |

ESP-IDF 6.0.2 remains the local-switch regression baseline. The Matter profile must use the matched toolchain above until a later baseline passes the same upstream and product gates.

The official Matter host flow is Linux. On Windows this project uses Ubuntu 24.04 under WSL2. `tools/build_matter_reference.sh` builds Espressif's upstream ESP32-C6 Thread reference. `tools/build_matter_node.sh` verifies the pinned Git SHAs before building this product.

## Build

```cmd
wsl -d Ubuntu-24.04 -u root -- bash /mnt/c/Users/lesli/WS/agile-smart-device/tools/build_matter_node.sh
```

The Matter build uses `sdkconfig.defaults.matter_node` and `partitions_matter.csv`. The 16 MiB layout reserves two 6 MiB OTA application slots, Matter/NVS storage, factory data, secure certificate data and coredump space. Local and Matter builds must run sequentially because both use the generated `managed_components/` directory; the Matter wrapper reconfigures before every build to restore its dependency set.

## Runtime ownership

`SmartDeviceApplication` and `BinarySwitchService` remain the single owner of relay state.

```text
GPIO9 ISR --------------------+
Matter OnOff PRE_UPDATE ------+--> SwitchRuntime queue --> SmartDeviceApplication
                                                    |--> GPIO10 relay
                                                    |--> GPIO2 relay LED
                                                    |--> delayed NVS state
                                                    +--> Matter OnOff attribute report
```

Rules:

- GPIO ISR and Matter callbacks only enqueue bounded events.
- Queue depth is eight events.
- Queue-full rejects a Matter write instead of silently dropping it.
- Duplicate desired state is idempotent and does not rewrite NVS.
- NVS writes are coalesced for 500 ms.
- Local control remains available while Thread/Controller/Gateway is unavailable.
- Matter attribute publication is scheduled onto the Matter stack context.

## Data model

| Item | Value |
|---|---|
| Root endpoint | 0 |
| Application endpoint | Dynamically assigned, currently 1 |
| Device type | On/Off Plug-in Unit `0x010A` |
| Server cluster | OnOff `0x0006` |
| State attribute | OnOff `0x0000` |
| Commands | Off `0x00`, On `0x01`, Toggle `0x02` |

The BBB must discover endpoint/Descriptor data and persist it in inventory. WebUI code must not assume endpoint 1 for every future SKU.

## Local lifecycle UX

| Input/status | Behavior |
|---|---|
| Short button press | Toggle relay. |
| Released hold of at least 5 seconds | Request/reopen commissioning window. |
| Released hold of at least 10 seconds | Request Matter factory reset. |
| GPIO2 LED | Mirror relay state only. |
| GPIO8 WS2812 amber | Commissioning window/open or fabric removed. |
| GPIO8 WS2812 blue | Thread/interface lifecycle transition. |
| GPIO8 WS2812 green | Commissioning complete. |
| GPIO8 WS2812 red | Commissioning fail-safe timeout. |

Long-press thresholds require physical HIL validation. Matter factory reset removes fabric/Thread credentials through ESP-Matter. The current product policy preserves the `smartdev` relay-state namespace.

## Verified evidence

Verified locally on ESP32-C6 revision v0.2:

- upstream ESP-Matter Thread reference builds under the pinned baseline;
- upstream image flashes and opens CHIPoBLE commissioning;
- product `matter_node` builds and flashes;
- endpoint 1 is created as On/Off Plug-in Unit;
- OpenThread and Matter server start;
- product image excludes the `imports/gateway_node` profile;
- local and Matter build profiles still compile;
- host/framework tests remain passing.

The current product image is approximately 1.7 MiB and leaves about 73% free in each 6 MiB OTA slot. This is a development baseline, not a production size target.

## BBB audit and open gate

Observed on BBB:

- `otbr-agent.service` is active;
- `wpan0` is up;
- OTBR role is leader;
- Matter Controller is matter.js 0.17.9;
- Gateway, WebUI and Mosquitto services are active;
- the Thread child table was empty during the audit.

The current Controller JSON-RPC surface exposes only `health`, `listNodes` and `invoke`. It does not yet expose commissioning, node removal, explicit read or subscription/event APIs. Therefore the final BBB gate remains open.

Before HIL completion, the BBB Controller must support:

1. commission using BLE plus the active Thread dataset;
2. remove/decommission node;
3. read endpoint/cluster attributes;
4. invoke OnOff commands;
5. subscribe to OnOff and forward local changes as Gateway events;
6. persist fabric, node inventory and subscriptions across restart.

A separate matter.js shell or commissioner may be used for temporary node bring-up, but it does not satisfy the final BBB fabric/end-to-end gate.

## Production blockers

The development image uses test commissioning/attestation material. Production release remains blocked on:

- assigned VID/PID and product identity;
- unique discriminator/passcode;
- DAC, PAI and Certification Declaration provisioning;
- secure boot, flash/NVS encryption and debug-port policy;
- signed Matter OTA, rollback and anti-rollback tests;
- controller commissioning/subscription API;
- power-cycle, brownout, Thread outage and soak-test evidence;
- physical confirmation that relay stays fail-safe OFF before GPIO initialization.
