from __future__ import annotations

from pathlib import Path
import subprocess
import sys


def test_authoritative_documentation() -> None:
    root = Path(__file__).resolve().parents[2]
    subprocess.run([sys.executable, str(root / "tools" / "check_docs.py")], cwd=root, check=True)
