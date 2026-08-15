#!/usr/bin/env python3
"""Enforce architecture, bounded-resource, and catalog-completeness rules."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
INCLUDE_PATTERN = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')
APPLICATION_FORBIDDEN = (
    "esp_", "esp32", "driver/", "freertos/", "nvs", "board/", "wifi", "mqtt",
    "lwip", "socket",
)
REUSABLE_FORBIDDEN = (
    "esp_", "esp32", "driver/", "freertos/", "nvs", "board/", "esp_wifi",
    "esp_ota", "esp_http", "esp_event", "mqtt_client", "lwip", "sys/socket",
    "mbedtls", "openssl", "cjson", "pthread",
)
DYNAMIC_FORBIDDEN = (
    r"\bnew\s", r"\bdelete\s", r"\bmalloc\s*\(", r"\bfree\s*\(",
    r"std::vector", r"std::string", r"std::function", r"std::shared_ptr",
    r"std::unique_ptr", r"#include\s*<string>", r"#include\s*<vector>",
    r"#include\s*<memory>", r"#include\s*<functional>", r"#include\s*<map>",
    r"#include\s*<thread>", r"#include\s*<mutex>",
)
SHARED_SERVICE_HEADERS = {
    "Limits.hpp", "DeviceIdentity.hpp", "MessageEnvelope.hpp", "SessionDescriptor.hpp",
    "NetworkTypes.hpp", "HealthTypes.hpp", "IndicationTypes.hpp", "CommandTypes.hpp",
    "OtaTypes.hpp", "TimeTypes.hpp", "TelemetryTypes.hpp",
}


def add_error(errors: list[str], root: Path, path: Path, line: int, rule: str) -> None:
    errors.append(f"{path.relative_to(root)}:{line}: {rule}")


def source_files(area: Path):
    if not area.exists():
        return
    for path in area.rglob("*"):
        if path.is_file() and (path.suffix.lower() in SOURCE_SUFFIXES or path.name == "CMakeLists.txt"):
            yield path


def reusable_areas(root: Path) -> tuple[Path, ...]:
    framework = root / "external" / "agile-firmware-framework" / "components"
    return tuple(framework / name for name in ("services", "libraries", "protocols", "devices", "uhal"))


def check_includes(root: Path, path: Path, forbidden: tuple[str, ...], rule: str,
                   errors: list[str]) -> None:
    if not path.exists():
        add_error(errors, root, path, 0, "required architecture file is missing")
        return
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        match = INCLUDE_PATTERN.match(line)
        if match is None:
            continue
        include = match.group(1).lower()
        if any(token in include for token in forbidden):
            add_error(errors, root, path, line_number, f"{rule}: forbidden include <{match.group(1)}>")


def check_boundaries(root: Path, errors: list[str]) -> None:
    application_files = (
        root / "components" / "product_smart_device" / "include" / "smart_device" /
        "SmartDeviceApplication.hpp",
        root / "components" / "product_smart_device" / "src" / "application" /
        "SmartDeviceApplication.cpp",
    )
    for path in application_files:
        check_includes(root, path, APPLICATION_FORBIDDEN, "Layer 5 application boundary", errors)

    for area in reusable_areas(root):
        for path in source_files(area):
            if path.name == "CMakeLists.txt":
                for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
                    lowered = line.lower()
                    if any(token in lowered for token in REUSABLE_FORBIDDEN):
                        add_error(errors, root, path, line_number,
                                  "Reusable CMake must not depend on vendor/RTOS/board components")
            else:
                check_includes(root, path, REUSABLE_FORBIDDEN, "Reusable component boundary", errors)

            if "components/services/" in path.as_posix() and path.suffix.lower() in SOURCE_SUFFIXES:
                own_component = path.relative_to(area).parts[0]
                for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
                    match = INCLUDE_PATTERN.match(line)
                    if match is None or not match.group(1).startswith("services/"):
                        continue
                    basename = Path(match.group(1)).name
                    if basename.startswith("I") or basename in SHARED_SERVICE_HEADERS:
                        continue
                    own_header = area / own_component / "include" / "services" / basename
                    if not own_header.exists():
                        add_error(errors, root, path, line_number,
                                  f"Concrete service-to-service include is forbidden: <{match.group(1)}>")

    entry = root / "main" / "main.cpp"
    allowed_main_includes = {"smart_device/SmartDevice.hpp", "esp_log.h"}
    if not entry.exists():
        add_error(errors, root, entry, 0, "required platform entry file is missing")
    else:
        for line_number, line in enumerate(entry.read_text(encoding="utf-8").splitlines(), start=1):
            match = INCLUDE_PATTERN.match(line)
            if match is not None and match.group(1) not in allowed_main_includes:
                add_error(errors, root, entry, line_number,
                          f"main may include only product entry API and logging, found <{match.group(1)}>")
            if any(token in line for token in
                   ("board::", "adapters::", "BinarySwitchService", "SmartDeviceApplication", "Nvs")):
                add_error(errors, root, entry, line_number,
                          "main must not construct or access product internals")


def check_dynamic_allocation(root: Path, errors: list[str]) -> None:
    framework = root / "external" / "agile-firmware-framework" / "components"
    for name in ("services", "libraries", "protocols", "devices"):
        for path in source_files(framework / name):
            if path.name == "CMakeLists.txt":
                continue
            for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
                if any(re.search(pattern, line) is not None for pattern in DYNAMIC_FORBIDDEN):
                    add_error(errors, root, path, line_number,
                              "Dynamic or unbounded allocation/type is forbidden in reusable code")


def direct_components(area: Path):
    if not area.exists():
        return
    for child in area.iterdir():
        if child.is_dir() and (child / "CMakeLists.txt").exists():
            yield child


def check_catalog(root: Path, errors: list[str]) -> None:
    framework = root / "external" / "agile-firmware-framework"
    test_cmake = (framework / "tests" / "unit" / "CMakeLists.txt").read_text(encoding="utf-8")
    for area_name in ("services", "libraries", "protocols", "devices"):
        area = framework / "components" / area_name
        for component in direct_components(area):
            source_or_header = any(
                path.is_file() and path.suffix.lower() in SOURCE_SUFFIXES
                for path in component.rglob("*")
            )
            if not source_or_header:
                continue
            readme = component / "README.md"
            if not readme.exists():
                add_error(errors, root, readme, 0, "implemented catalog component requires README.md")
            cmake_text = (component / "CMakeLists.txt").read_text(encoding="utf-8")
            match = re.search(r"add_library\s*\(\s*([A-Za-z0-9_\-]+)", cmake_text)
            if match is None:
                add_error(errors, root, component / "CMakeLists.txt", 1,
                          "implemented component requires an add_library target")
            elif match.group(1) not in test_cmake:
                add_error(errors, root, component / "CMakeLists.txt", 1,
                          f"target {match.group(1)} is not referenced by framework unit tests")


def check_cmake(root: Path, errors: list[str]) -> None:
    cmake_files = list((root / "components").glob("*/CMakeLists.txt"))
    cmake_files.extend((root / "imports").glob("*/components/*/CMakeLists.txt"))
    framework_components = root / "external" / "agile-firmware-framework" / "components"
    cmake_files.extend(framework_components.rglob("CMakeLists.txt"))

    source_suffixes = (".c", ".cc", ".cpp", ".cxx", ".S", ".s")
    for cmake in cmake_files:
        text = cmake.read_text(encoding="utf-8")
        for line_number, line in enumerate(text.splitlines(), start=1):
            if re.search(r"\b(?:file\s*\(\s*)?GLOB(?:_RECURSE)?\b", line, re.IGNORECASE):
                add_error(errors, root, cmake, line_number,
                          "Firmware components require explicit source lists; globbing is forbidden")

        for token in re.findall(r'"([^"]+)"', text):
            if not token.endswith(source_suffixes):
                continue
            if token.startswith("${FRAMEWORK_ROOT}/"):
                candidate = root / "external" / "agile-firmware-framework" / token.split("}/", 1)[1]
            elif token.startswith("${CMAKE_CURRENT_LIST_DIR}/"):
                candidate = cmake.parent / token.split("}/", 1)[1]
            elif "${" in token or "$<" in token:
                continue
            else:
                candidate = cmake.parent / token
            if not candidate.resolve().is_file():
                add_error(errors, root, cmake, 1,
                          f"CMake source path does not exist: {token}")

    product_cmake = root / "components" / "product_smart_device" / "CMakeLists.txt"
    if product_cmake.exists():
        text = product_cmake.read_text(encoding="utf-8")
        if 'INCLUDE_DIRS "include"' not in text:
            add_error(errors, root, product_cmake, 1,
                      "Product public headers must be exported only from include/")
        if 'PRIV_INCLUDE_DIRS "include"' in text:
            add_error(errors, root, product_cmake, 1,
                      "Product public include/ must not be declared private")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--check", choices=("all", "boundaries", "dynamic", "catalog", "cmake"), default="all")
    args = parser.parse_args()
    root = args.root.resolve()
    errors: list[str] = []

    if args.check in ("all", "boundaries"):
        check_boundaries(root, errors)
    if args.check in ("all", "dynamic"):
        check_dynamic_allocation(root, errors)
    if args.check in ("all", "catalog"):
        check_catalog(root, errors)
    if args.check in ("all", "cmake"):
        check_cmake(root, errors)

    if errors:
        print(f"Layer rule violations ({args.check}):")
        for error in errors:
            print(f"  {error}")
        return 1

    print(f"Layer rule check passed ({args.check})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
