# Embedded C++ Coding Standard

Target profile: C++17, ESP-IDF 6.0.2, exceptions disabled, RTTI disabled.

## Safety-oriented rules

- Prefer fixed-size storage and bounded queues. Avoid uncontrolled heap allocation after startup.
- Make ownership explicit. Use references for required injected dependencies and pointers only for optional/non-owning relationships.
- Initialize every object and field before use. Keep hardware in a safe state during partial initialization.
- Check and propagate status at system boundaries. Test failure behavior, not only success paths.
- Keep ISR work bounded, non-blocking, allocation-free, and storage-free.
- Avoid hidden global mutable state, unsafe casts, recursion, unbounded loops without a scheduling/blocking point, and macro-based control flow.
- Use explicit-width integer types for persisted, protocol, and hardware-facing data.
- Document timing, queue depth, task stack, polarity, and resource limits next to the owning component.

## SOLID and dependency injection

- SRP: separate application behavior, runtime scheduling, persistence adapter, board configuration, and reusable service policy.
- OCP: add transports/adapters behind narrow contracts rather than changing application behavior.
- LSP: every adapter must honor the same UHAL success/error semantics.
- ISP: add small capability interfaces; never create a broad `IHal` or `INetworkEverything`.
- DIP: application and services depend on contracts, while the composition root selects concrete implementations.

## MISRA-oriented use

These rules support MISRA-oriented authoring but are not formal compliance evidence. Formal claims require a configured qualified analyzer, reviewed deviations, complete build capture, and traceable reports. Use the detailed documents under `reference/docs/Common/Coding_Standards` as background; do not copy legacy C++03 rules blindly into this C++17 target.
