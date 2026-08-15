#!/usr/bin/env python3

import argparse
import importlib.util
import sys
import tempfile

sys.dont_write_bytecode = True
from pathlib import Path


def load_checker(path: Path):
    spec = importlib.util.spec_from_file_location("layer_checker", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--checker", type=Path, required=True)
    args = parser.parse_args()
    checker = load_checker(args.checker.resolve())

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        component = root / "components" / "sample"
        component.mkdir(parents=True)
        (component / "source.cpp").write_text("int sample() { return 0; }\n", encoding="utf-8")
        (component / "CMakeLists.txt").write_text(
            'idf_component_register(SRCS "source.cpp")\n', encoding="utf-8")
        errors = []
        checker.check_cmake(root, errors)
        if errors:
            print("Valid CMake fixture was rejected:", errors)
            return 1

        (component / "CMakeLists.txt").write_text(
            'file(GLOB_RECURSE SOURCES "*.cpp")\n'
            'idf_component_register(SRCS "missing.cpp")\n', encoding="utf-8")
        errors = []
        checker.check_cmake(root, errors)
        messages = "\n".join(errors)
        if "globbing is forbidden" not in messages or "does not exist" not in messages:
            print("Invalid CMake fixture did not trigger expected rules:", errors)
            return 1

    print("Layer checker fixture tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
