# Smart Device Product

See the [firmware handbook](../../docs/handbook/04-firmware-node.md) and [Matter/Thread chapter](../../docs/handbook/05-matter-thread.md) for end-to-end behavior.

This ESP-IDF component owns the Layer 5 application, composition root, runtime, product persistence, and optional Matter-over-Thread infrastructure for the smart-device firmware.

```text
main
  -> SmartDevice composition root
      -> SmartDeviceApplication (vendor-neutral product use cases)
          -> BinarySwitchService (Layer 4 policy)
      -> SwitchRuntime (bounded FreeRTOS event queue and button event pump)
      -> Board and NvsBinaryStateStore (concrete infrastructure)
      -> MatterNode (matter_node profile only: OnOff endpoint, Thread lifecycle, WS2812)
```

`SmartDeviceApplication` owns semantic product behavior such as initialize, short-press handling, explicit switch commands, and state queries. It may depend on Layer 4 services and UHAL status only.

Composition/runtime infrastructure owns FreeRTOS tasks and ISR handoff, board construction, NVS initialization, and the product persistence schema. The NVS adapter uses namespace `smartdev`, schema key `schema` version `1`, and state key `relay_on`; writes are coalesced for 500 ms and unchanged commands do not rewrite state.

Reusable on/off policy lives in `services::BinarySwitchService`. `NvsBinaryStateStore` implements the service-owned `services::IBinaryStateStore` port; Layer 4 remains independent from ESP-IDF, FreeRTOS, board pins, and NVS details.

With `PRODUCT_PROFILE=matter_node`, `MatterNode` creates endpoint 1 as Matter On/Off Plug-in Unit. Matter writes are validated and posted to the same bounded runtime queue used by local control. Application state changes are scheduled back onto the Matter stack for attribute reporting. MQTT/JSON Gateway contracts are not compiled into the node. GPIO8 WS2812 reports lifecycle state while GPIO2 continues to mirror the relay. A 5-second released hold requests a commissioning window and a 10-second released hold requests Matter factory reset. Product relay state is preserved across Matter factory reset by the current policy.
