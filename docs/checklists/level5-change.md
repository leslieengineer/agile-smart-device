# Level 5 Change Checklist

- [ ] Product behavior is implemented in `SmartDeviceApplication`, not in ISR/task/vendor callbacks.
- [ ] Application includes only Layer 4 services, UHAL status, and pure product value types.
- [ ] Composition root is the only code creating concrete adapters/services/runtime objects.
- [ ] Runtime converts hardware/RTOS events into semantic application calls.
- [ ] Board pins, polarity, credentials, NVS schema, and transport configuration stay outside application logic.
- [ ] All dependencies use constructor injection with explicit lifetime ownership.
- [ ] Success, failure, offline, and partial-initialization behavior has tests.
- [ ] Queue depth, task stack, timing, and memory bounds are explicit.
- [ ] New third-party dependencies satisfy `docs/rules/dependencies.md`.
- [ ] `tools/check_layer_boundaries.py` and host CTest pass.
- [ ] ESP-IDF build passes without adding vendor includes to application or Layer 4.
- [ ] Authoritative chapter/status/runbook affected by the behavior is updated.
- [ ] `docs/checklists/doc-change.md` and `python3 tools/check_docs.py` pass.
- [ ] No MISRA/compliance claim is made without analyzer evidence and deviations.
