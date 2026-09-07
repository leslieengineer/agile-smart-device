# Firmware Diagrams

The firmware CI generates the component dependency graph from the checked-in
CMake metadata. Generated files are CI artifacts and are not committed to the
source tree.

The first generated artifact is `firmware-components.mmd`. It is deterministic:
the same source tree produces the same Mermaid text, so later generators can
add freshness checks without creating noisy diffs.

`spec/firmware-flows.json` is the source for runtime flows that cannot be
reliably inferred from CMake alone. CI checks every referenced file and symbol,
then generates `firmware-flows.md`. Update the spec only when the runtime
meaning of a flow changes; do not edit generated files.

The generated graph describes build-time component relationships. It does not
replace the authoritative layer rules, runtime behavior, HIL evidence, or
sequence/state-machine specifications in the main documentation.

On a push to `main`, CI publishes the generated site to GitHub Pages. Pull
requests receive the same files as workflow artifacts without changing the
published site. The Pages site is the rendered view; the Mermaid and Markdown
files remain available in the artifact for review and diffing.