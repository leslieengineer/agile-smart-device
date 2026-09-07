#!/usr/bin/env python3

import importlib.util
import sys
import tempfile
from pathlib import Path


def load_validator(path: Path):
    spec = importlib.util.spec_from_file_location("diagram_spec_validator", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def main() -> int:
    validator = load_validator(Path(__file__).resolve().parents[2] / "tools" / "validate_diagram_specs.py")
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "flow.cpp"
        source.write_text("void Flow::start() {}\n", encoding="utf-8")
        spec = {"flows": [{"id": "flow", "title": "Flow", "nodes": [
            {"id": "source", "file": "flow.cpp", "symbol": "Flow::start"}], "edges": []}]}
        if validator.validate(spec, root):
            print("Valid diagram spec was rejected")
            return 1
        spec["flows"][0]["nodes"][0]["symbol"] = "Flow::missing"
        errors = validator.validate(spec, root)
        if not errors or "symbol not found" not in errors[0]:
            print("Stale symbol was not rejected")
            return 1
    print("Diagram specification fixture tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())