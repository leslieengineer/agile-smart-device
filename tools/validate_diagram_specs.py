#!/usr/bin/env python3
"""Validate code-backed flow specs and render Mermaid sequence diagrams."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def validate(spec: dict, root: Path) -> list[str]:
    errors: list[str] = []
    flow_ids: set[str] = set()
    for flow in spec.get("flows", []):
        flow_id = flow.get("id", "")
        if not flow_id or flow_id in flow_ids:
            errors.append(f"duplicate or missing flow id: {flow_id!r}")
        flow_ids.add(flow_id)
        node_ids: set[str] = set()
        for node in flow.get("nodes", []):
            node_id = node.get("id", "")
            if not node_id or node_id in node_ids:
                errors.append(f"{flow_id}: duplicate or missing node id: {node_id!r}")
            node_ids.add(node_id)
            path = root / node.get("file", "")
            if not path.is_file():
                errors.append(f"{flow_id}/{node_id}: file does not exist: {node.get('file', '')}")
                continue
            if node.get("symbol", "") not in path.read_text(encoding="utf-8"):
                errors.append(f"{flow_id}/{node_id}: symbol not found: {node.get('symbol', '')}")
        for edge in flow.get("edges", []):
            if edge.get("from") not in node_ids or edge.get("to") not in node_ids:
                errors.append(f"{flow_id}: edge references an unknown node")
    return errors


def render(spec: dict) -> str:
    lines: list[str] = ["<!-- GENERATED FILE - DO NOT EDIT.",
                        "Source: docs/diagrams/spec/firmware-flows.json -->"]
    for flow in spec.get("flows", []):
        lines.extend(["", f"## {flow['title']}", "", "```mermaid", "sequenceDiagram"])
        for node in flow["nodes"]:
            lines.append(f"    participant {node['id']} as {node['symbol']}")
        for edge in flow.get("edges", []):
            lines.append(f"    {edge['from']}->>{edge['to']}: {edge['label']}")
        lines.append("```")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--spec", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    spec = json.loads(args.spec.read_text(encoding="utf-8"))
    errors = validate(spec, root)
    if errors:
        print("Diagram specification validation failed:")
        print("\n".join(f"- {error}" for error in errors))
        return 1
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(render(spec), encoding="utf-8")
    print(f"Diagram specification validation passed ({len(spec.get('flows', []))} flows)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())