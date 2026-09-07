#!/usr/bin/env python3
"""Build a small static documentation site for GitHub Pages."""

from __future__ import annotations

import argparse
import html
import json
import re
from pathlib import Path

from generate_firmware_diagrams import collect, render as render_components
from validate_diagram_specs import render as render_flows


MERMAID_BLOCK = re.compile(r"```mermaid\n(.*?)```", re.DOTALL)


def build(root: Path, output: Path) -> None:
    output.mkdir(parents=True, exist_ok=True)
    diagrams = output / "diagrams"
    diagrams.mkdir(exist_ok=True)
    component_graph = render_components(collect(root), "CMakeLists.txt")
    spec_path = root / "docs" / "diagrams" / "spec" / "firmware-flows.json"
    flow_graph = render_flows(json.loads(spec_path.read_text(encoding="utf-8")))
    diagrams.joinpath("firmware-components.mmd").write_text(component_graph, encoding="utf-8")
    diagrams.joinpath("firmware-flows.md").write_text(flow_graph, encoding="utf-8")

    component_mermaid = "\n".join(component_graph.splitlines()[2:])
    flow_titles = re.findall(r"^## (.+)$", flow_graph, re.MULTILINE)
    flow_blocks = MERMAID_BLOCK.findall(flow_graph)
    flow_html = "\n".join(
        f'<section><h2>{html.escape(title)}</h2><div class="mermaid">{html.escape(block)}</div></section>'
      for title, block in zip(flow_titles, flow_blocks)
    )
    page = f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Agile Smart Device | Firmware Architecture</title>
  <style>
    :root {{ color-scheme: light; font-family: system-ui, sans-serif; }}
    body {{ margin: 0; background: #f4f1ea; color: #20231f; }}
    main {{ max-width: 1180px; margin: 0 auto; padding: 40px 22px 72px; }}
    header {{ border-bottom: 3px solid #20231f; margin-bottom: 30px; }}
    h1 {{ font-size: clamp(2rem, 5vw, 4.4rem); line-height: 1; margin: 0 0 14px; }}
    h2 {{ margin-top: 34px; }}
    .lead {{ max-width: 680px; font-size: 1.08rem; line-height: 1.6; }}
    section {{ background: #fffdf8; border: 1px solid #c9c5bb; padding: 22px; margin: 22px 0; overflow-x: auto; }}
    .mermaid {{ min-width: 680px; }}
    a {{ color: #0b5d58; }}
    code {{ background: #e8e4da; padding: 2px 5px; }}
  </style>
  <script type="module">
    import mermaid from 'https://cdn.jsdelivr.net/npm/mermaid@11/dist/mermaid.esm.min.mjs';
    mermaid.initialize({{ startOnLoad: true, theme: 'base', themeVariables: {{ primaryColor: '#d9ebe2', lineColor: '#20231f' }} }});
  </script>
</head>
<body>
<main>
  <header>
    <p>AGILE SMART DEVICE / GENERATED DOCUMENTATION</p>
    <h1>Firmware architecture</h1>
    <p class="lead">Generated from firmware CMake metadata and code-backed runtime flow specifications. Refresh this page from CI after every source change.</p>
  </header>
  <section>
    <h2>Component and layer dependencies</h2>
    <div class="mermaid">{html.escape(component_mermaid)}</div>
    <p>Source: <a href="diagrams/firmware-components.mmd">firmware-components.mmd</a></p>
  </section>
  {flow_html}
  <p>Runtime flow source: <code>docs/diagrams/spec/firmware-flows.json</code>. Generated output is validated against referenced files and symbols.</p>
</main>
</body>
</html>
"""
    (output / "index.html").write_text(page, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    build(args.root.resolve(), args.output.resolve())
    print(f"Built documentation site at {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
