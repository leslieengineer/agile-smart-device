# AI Engineering Contract

Before changing firmware, read:

1. `docs/README.md` for authoritative system status and navigation
2. `docs/rules/architecture.md`
3. `docs/rules/coding-standards.md`
4. `docs/rules/dependencies.md`
5. `docs/checklists/level5-change.md` for product/application work
6. `docs/checklists/doc-change.md` before changing system behavior or documentation

Required behavior:

- Inspect existing code and dependency direction before proposing edits.
- Never bypass a layer to call a vendor API from application or reusable logic.
- Keep changes limited to the requested vertical slice.
- Add or update tests for every behavior change and failure path.
- Prefer existing ESP-IDF, FreeRTOS, lwIP, coreMQTT, and proven libraries over custom replacements when they satisfy the requirement.
- Record architectural or standards deviations explicitly.
- Do not claim MISRA, CERT, or other formal compliance without analyzer output and reviewed deviation evidence.
- Do not stage, commit, push, flash, erase, or reset hardware unless explicitly requested.
