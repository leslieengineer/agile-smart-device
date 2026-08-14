# Architecture Rules

## Dependency direction

```text
Layer 5 product/application -> Layer 4 reusable logic -> Layer 2 UHAL contracts
Layer 5 composition         -> Layer 3 adapters       -> Layer 1 low-level -> vendor SDK
```

| Area | Owns | May depend on | Forbidden |
|---|---|---|---|
| Layer 1 low-level | Vendor calls, registers, IRQ primitives | Vendor SDK/RTOS | Product policy |
| Layer 2 UHAL | Small platform-neutral capability contracts | UHAL core | Vendor types, board facts |
| Layer 3 adapters | Map Layer 1/vendor errors to UHAL | Layer 1, UHAL | Device protocol, product policy |
| Layer 4 logic | Reusable devices, protocols, services, pure libraries | UHAL, pure libraries | Vendor SDK, RTOS, board, NVS schema |
| Layer 5 application | Product use cases and semantic commands | Layer 4, product value types | Vendor SDK, RTOS, board pins, concrete adapters |
| Layer 5 composition/runtime | Concrete object graph, scheduling, persistence adapters | All selected components | Reusable business policy |

## Mandatory rules

- `SmartDeviceApplication.hpp/.cpp` must not include ESP-IDF, FreeRTOS, NVS, board, transport, or concrete-adapter headers.
- `main/app_main()` only invokes the product composition-root API and reports startup status.
- Only the composition root creates concrete adapters and injects dependencies.
- ISR code only captures hardware events and wakes a task; it never executes service policy, storage, logging, or blocking APIs.
- Board pin, polarity, clock, partition, and wiring facts stay in board/product configuration.
- Layer 4 services own policy-facing ports; Layer 5 supplies storage/transport implementations.
- No service locator or mutable singleton API. Use constructor injection and explicit lifetime ownership.
