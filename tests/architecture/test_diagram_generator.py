#!/usr/bin/env python3

import importlib.util
import sys
import tempfile
from pathlib import Path


def load_generator(path: Path):
    spec = importlib.util.spec_from_file_location("diagram_generator", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def main() -> int:
    generator = load_generator(Path(__file__).resolve().parents[2] / "tools" / "generate_firmware_diagrams.py")
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        product = root / "components" / "product_smart_device"
        framework = root / "external" / "agile-firmware-framework" / "components" / "services" / "switch"
        product.mkdir(parents=True)
        framework.mkdir(parents=True)
        (product / "CMakeLists.txt").write_text(
            'set(PRODUCT_REQUIRES framework_switch)\n'
            'idf_component_register(REQUIRES ${PRODUCT_REQUIRES})\n', encoding="utf-8")
        (framework / "CMakeLists.txt").write_text(
            'add_library(framework_switch STATIC src.cpp)\n', encoding="utf-8")
        components = generator.collect(root)
        output = generator.render(components, "CMakeLists.txt")
        if "product_smart_device" not in output or "framework_switch" not in output:
            print("Expected component nodes were not generated")
            return 1
        if "product_smart_device --> framework_switch" not in output:
            if "product_smart_device --> switch" not in output:
                print("Expected dependency edge was not generated")
                return 1
        if 'subgraph Layer5["Layer 5"]' not in output:
            print("Layer subgraph was not generated")
            return 1
        if output != generator.render(generator.collect(root), "CMakeLists.txt"):
            print("Generator output is not deterministic")
            return 1
    print("Diagram generator fixture tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())