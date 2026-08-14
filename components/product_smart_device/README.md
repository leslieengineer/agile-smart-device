# Smart Device Product

This ESP-IDF component owns the Layer 5 application, composition root, and runtime for the smart-device firmware.

```text
main
  -> SmartDevice composition root
      -> SmartDeviceApplication (vendor-neutral product use cases)
          -> BinarySwitchService (Layer 4 policy)
      -> SwitchRuntime (FreeRTOS and button event pump)
      -> Board and NvsBinaryStateStore (concrete infrastructure)
```

`SmartDeviceApplication` owns semantic product behavior such as initialize, short-press handling, explicit switch commands, and state queries. It may depend on Layer 4 services and UHAL status only.

Composition/runtime infrastructure owns FreeRTOS tasks and ISR handoff, board construction, NVS initialization, and the product persistence schema. The NVS adapter uses namespace `smartdev`, schema key `schema` version `1`, and state key `relay_on`.

Reusable on/off policy lives in `services::BinarySwitchService`. `NvsBinaryStateStore` implements the service-owned `services::IBinaryStateStore` port; Layer 4 remains independent from ESP-IDF, FreeRTOS, board pins, and NVS details.
