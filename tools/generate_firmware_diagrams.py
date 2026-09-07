#!/usr/bin/env python3
"""Generate deterministic firmware component diagrams from CMake metadata."""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path


TARGET = re.compile(r"\badd_library\s*\(\s*([A-Za-z0-9_.:-]+)", re.IGNORECASE)
LINKS = re.compile(r"\btarget_link_libraries\s*\(\s*[^\s)]+\s+(.*?)\)", re.IGNORECASE | re.DOTALL)
REGISTER = re.compile(r"\bidf_component_register\s*\((.*?)\)", re.IGNORECASE | re.DOTALL)
SET_VARIABLE = re.compile(r"\bset\s*\(\s*([A-Z][A-Z0-9_]*)\s+([^)]*)\)", re.IGNORECASE | re.DOTALL)
TOKEN = re.compile(r"[A-Za-z0-9_.:${}/+\-]+")
COMMON_COMPONENTS = {
    "bounded_serialization", "fixed_ring_buffer", "retry_policy", "services_types",
    "uhal_core", "uhal_libraries", "uhal_services",
}
LAYER_ORDER = ("Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Common / Utilities", "Other")


@dataclass(frozen=True)
class Component:
    name: str
    path: str
    layer: str
    dependencies: tuple[str, ...]


def tokens(value: str) -> list[str]:
    return [token.strip('"') for token in TOKEN.findall(value) if not token.startswith("$")]


def expand(value: str, variables: dict[str, list[str]]) -> list[str]:
    expanded = value
    for name, values in variables.items():
        expanded = expanded.replace("${" + name + "}", " ".join(values))
    return tokens(expanded)


def layer_for(path: Path, root: Path) -> str:
    relative = path.parent.relative_to(root).as_posix()
    if relative.startswith("components/product_"):
        return "Layer 5"
    if relative.startswith("components/board_"):
        return "Layer 3"
    if "/components/uhal/" in "/" + relative:
        return "Layer 2"
    if "/components/platform/" in "/" + relative:
        return "Layer 1"
    if "/components/" in "/" + relative:
        return "Layer 4"
    return "Other"


def parse_cmake(path: Path, root: Path) -> Component | None:
    text = path.read_text(encoding="utf-8")
    variables = {match.group(1): tokens(match.group(2)) for match in SET_VARIABLE.finditer(text)}
    target_match = TARGET.search(text)
    if target_match:
        name = target_match.group(1)
        dependencies = []
        for match in LINKS.finditer(text):
            dependencies.extend(token for token in tokens(match.group(1))
                               if token.upper() not in {"PUBLIC", "PRIVATE", "INTERFACE"})
    else:
        register_match = REGISTER.search(text)
        if register_match is None:
            return None
        name = path.parent.name
        register = register_match.group(1)
        dependencies = []
        requires = re.search(
            r"\bREQUIRES\s+(.+?)(?=\b(?:PRIV_REQUIRES|INCLUDE_DIRS|PRIV_INCLUDE_DIRS)\b|$)",
            register, re.IGNORECASE | re.DOTALL)
        private_requires = re.search(r"\bPRIV_REQUIRES\s+(.+)$", register,
                                     re.IGNORECASE | re.DOTALL)
        if requires:
            dependencies.extend(expand(requires.group(1), variables))
        if private_requires:
            dependencies.extend(expand(private_requires.group(1), variables))

    return Component(name, path.parent.relative_to(root).as_posix(), layer_for(path, root),
                     tuple(sorted(set(dependencies))))


def collect(root: Path) -> list[Component]:
    paths = sorted(root.glob("components/**/CMakeLists.txt"))
    paths.extend(sorted((root / "external" / "agile-firmware-framework" / "components")
                        .glob("**/CMakeLists.txt")))
    components = [component for path in paths if (component := parse_cmake(path, root))]
    return sorted(components, key=lambda component: (component.layer, component.name, component.path))


def render(components: list[Component], source: str) -> str:
    lines = ["%% GENERATED FILE - DO NOT EDIT.", f"%% Source: {source}", "flowchart LR"]
    groups: dict[str, list[Component]] = {layer: [] for layer in LAYER_ORDER}
    for component in components:
        layer = "Common / Utilities" if component.name in COMMON_COMPONENTS else component.layer
        groups.setdefault(layer, []).append(component)

    for layer in LAYER_ORDER:
        if not groups[layer]:
            continue
        group_id = re.sub(r"[^A-Za-z0-9]", "", layer)
        lines.append(f'    subgraph {group_id}["{layer}"]')
        for component in groups[layer]:
            node = component.name.replace("-", "_").replace(".", "_")
            lines.append(f'        {node}["{component.name}"]')
        lines.append("    end")

    known = {component.name for component in components}
    def resolve(dependency: str) -> str:
        if dependency in known:
            return dependency
        alias = dependency.removeprefix("framework_")
        return alias if alias in known else dependency

    def visible(component: Component, dependency: str) -> bool:
        target = next((item for item in components if item.name == dependency), None)
        if target is None or target.name in COMMON_COMPONENTS:
            return False
        source_is_business = component.layer in {"Layer 4", "Layer 5"}
        target_is_boundary = target.layer in {"Layer 1", "Layer 2", "Layer 3"}
        return source_is_business and (target_is_boundary or target.layer in {"Layer 4", "Layer 5"})

    edges = sorted((component.name, resolve(dependency)) for component in components
                   for dependency in component.dependencies
                   if resolve(dependency) in known
                   and resolve(dependency) != component.name
                   and visible(component, resolve(dependency)))
    for source_name, dependency in edges:
        source_node = source_name.replace("-", "_").replace(".", "_")
        dependency_node = dependency.replace("-", "_").replace(".", "_")
        lines.append(f"    {source_node} --> {dependency_node}")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = args.root.resolve()
    components = collect(root)
    if not components:
        raise SystemExit("No firmware CMake components found")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render(components, "CMakeLists.txt"), encoding="utf-8")
    print(f"Generated {args.output} ({len(components)} components)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())