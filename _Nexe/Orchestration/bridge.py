#!/usr/bin/env python3
"""Stable process boundary between the Cobra CLI and the C++ build engine."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
TEMPLATES = Path(__file__).resolve().parent / "templates"


def engine_binary(explicit: str | None) -> str:
    """Return an executable engine path, building it with CMake only if needed."""
    candidates = [explicit, os.environ.get("NEXE_ENGINE"), ROOT / "build" / "nexe_engine", shutil.which("nexe_engine")]
    for candidate in candidates:
        if candidate and Path(candidate).is_file():
            return str(candidate)

    build_dir = ROOT / "build"
    subprocess.run(["cmake", "-S", str(ROOT), "-B", str(build_dir)], check=True)
    subprocess.run(["cmake", "--build", str(build_dir)], check=True)
    built = build_dir / "nexe_engine"
    if not built.is_file():
        raise RuntimeError("CMake completed but did not create nexe_engine")
    return str(built)


def run_engine(args: argparse.Namespace) -> int:
    engine = engine_binary(args.engine)
    return subprocess.run([engine, args.command, args.target], check=False).returncode


def initialise(directory: str) -> int:
    destination = Path(directory).resolve()
    destination.mkdir(parents=True, exist_ok=True)
    # BUILD.sbl is the default. Keep the TOML template so projects that build
    # on it are not forced to migrate at once.
    for name in ("BUILD.sbl", "build.toml", "hello.c"):
        source, target = TEMPLATES / name, destination / name
        if target.exists():
            raise RuntimeError(f"refusing to overwrite {target}")
        shutil.copyfile(source, target)
        print(f"created {target}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Nexe orchestration bridge")
    parser.add_argument("command", choices=("build", "hash", "init"))
    parser.add_argument("target", help="manifest/file for build or hash; destination for init")
    parser.add_argument("--engine", help="path to an existing nexe_engine executable")
    args = parser.parse_args()
    try:
        return initialise(args.target) if args.command == "init" else run_engine(args)
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f"nexe bridge: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
