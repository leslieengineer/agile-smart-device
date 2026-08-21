#!/usr/bin/env python3
"""Validate authoritative documentation links, status tables and secret hygiene."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / "docs"
CHAPTERS = [DOCS / f"{index:02d}-{name}.md" for index, name in enumerate([
    "tong-quan-san-pham",
    "kien-truc-end-to-end",
    "firmware-esp32c6",
    "rhophi-claim-gatt",
    "mobile-commissioning",
    "bbb-gateway-controller-webui",
    "thread-ipv6-routing",
    "contract-api-mqtt-sse",
    "build-flash-deploy",
    "security-manufacturing-secrets",
    "testing-hil-evidence",
    "operations-troubleshooting",
    "status-open-issues",
    "thuat-ngu-ownership-repository",
])]
AUTHORITATIVE = [
    DOCS / "README.md",
    *CHAPTERS,
    *(DOCS / "runbooks").glob("*.md"),
    *(DOCS / "guides").glob("*.md"),
]
LEGACY_ENTRYPOINTS = [
    DOCS / "handbook/00-bat-dau.md",
    DOCS / "full-context/README.md",
    *(DOCS / "architecture").glob("*.md"),
    ROOT / "dashboard-reference/docs/README.md",
    ROOT / "mobileapp-reference/mobile-app/context.md",
    ROOT / "reference/README.md",
    ROOT / "Refactor_plan.md",
]
LINK = re.compile(r"\[[^]]+\]\(([^)]+)\)")
LONG_HEX = re.compile(r"(?i)\b[0-9a-f]{100,}\b")
BANNED = ("C:\\\\Users", "wsl -d", "ESP-IDF v5.5.5", "release/v1.6")


def fail(errors: list[str], path: Path, message: str) -> None:
    errors.append(f"{path.relative_to(ROOT)}: {message}")


def main() -> int:
    errors: list[str] = []
    index = (DOCS / "README.md").read_text(encoding="utf-8")
    for path in AUTHORITATIVE:
        if not path.is_file():
            fail(errors, path, "missing authoritative file")
            continue
        text = path.read_text(encoding="utf-8")
        if path in CHAPTERS and "| Source |" not in text:
            fail(errors, path, "missing Source/Deployed/HIL status table")
        for banned in BANNED:
            if banned in text:
                fail(errors, path, f"contains banned stale pattern {banned!r}")
        if LONG_HEX.search(text):
            fail(errors, path, "contains a long hexadecimal value that may be a secret")
        for match in LINK.finditer(text):
            target = match.group(1).split("#", 1)[0]
            if not target or target.startswith(("http://", "https://", "mailto:")):
                continue
            resolved = (path.parent / target).resolve()
            if not resolved.exists():
                fail(errors, path, f"broken link {target}")
    for chapter in CHAPTERS:
        if chapter.name not in index:
            fail(errors, DOCS / "README.md", f"chapter not indexed: {chapter.name}")
    for path in LEGACY_ENTRYPOINTS:
        if path.is_file() and "LEGACY / NON-AUTHORITATIVE" not in path.read_text(encoding="utf-8")[:800]:
            fail(errors, path, "missing legacy banner near file start")
    if errors:
        print("Documentation validation failed:")
        print("\n".join(f"- {error}" for error in errors))
        return 1
    print(f"Documentation validation passed ({len(AUTHORITATIVE)} authoritative files)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
