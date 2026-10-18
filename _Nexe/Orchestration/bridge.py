# This is the project form creator Chirag Balakrishna.
#  This project is licensed under GPLv3
# Thats it ! Feel free to contribute !! Happy Journey 😀


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
    executable = "nexe_engine.exe" if os.name == "nt" else "nexe_engine"
    candidates = [
        explicit,
        os.environ.get("NEXE_ENGINE"),
        ROOT / "build" / executable,
        ROOT / "build" / "Release" / executable,
        shutil.which(executable),
    ]
    for candidate in candidates:
        if candidate and Path(candidate).is_file():
            return str(candidate)

    build_dir = ROOT / "build"
    subprocess.run(["cmake", "-S", str(ROOT), "-B", str(build_dir)], check=True)
    subprocess.run(["cmake", "--build", str(build_dir)], check=True)
    for built in (build_dir / executable, build_dir / "Release" / executable):
        if built.is_file():
            return str(built)
    raise RuntimeError(f"CMake completed but did not create {executable}")


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
    buildstream_source = TEMPLATES / "buildstream"
    buildstream_target = destination / "buildstream"
    if buildstream_target.exists():
        raise RuntimeError(f"refusing to overwrite {buildstream_target}")
    shutil.copytree(buildstream_source, buildstream_target)
    print(f"created {buildstream_target}")
    return 0


def render_buildstream(directory: str) -> int:
    """Run a side-effect-free Starlark template and materialize only .bst files."""
    template_root = Path(directory).resolve()
    script = template_root / "render.star"
    if not script.is_file():
        raise RuntimeError(f"missing Starlark entrypoint {script}")
    interpreter = os.environ.get("STARLARK") or shutil.which("starlark")
    if not interpreter:
        raise RuntimeError("Starlark interpreter not found; set STARLARK or install github.com/canonical/starlark/cmd/starlark")
    result = subprocess.run(
        [interpreter, str(script)], cwd=template_root, check=True, text=True,
        capture_output=True, timeout=30,
    )
    elements: dict[Path, list[str]] = {}
    current: Path | None = None
    for line in result.stdout.splitlines():
        if line.startswith("@@NEXE_BST "):
            relative = Path(line.removeprefix("@@NEXE_BST "))
            if relative.suffix != ".bst" or relative.is_absolute() or ".." in relative.parts:
                raise RuntimeError(f"Starlark emitted unsafe element path: {relative}")
            if relative in elements:
                raise RuntimeError(f"Starlark emitted duplicate element: {relative}")
            elements[relative] = []
            current = relative
        elif current is None:
            if line:
                raise RuntimeError("Starlark output must begin with an @@NEXE_BST header")
        else:
            elements[current].append(line)
    if not elements:
        raise RuntimeError("Starlark template emitted no BuildStream elements")
    for relative, lines in elements.items():
        destination = template_root / "elements" / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text("\n".join(lines) + "\n", encoding="utf-8")
        print(f"rendered {destination}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Nexe orchestration bridge")
    parser.add_argument("command", choices=("build", "hash", "init", "render-bst"))
    parser.add_argument("target", help="manifest/file, init destination, or BuildStream template directory")
    parser.add_argument("--engine", help="path to an existing nexe_engine executable")
    args = parser.parse_args()
    try:
        if args.command == "init":
            return initialise(args.target)
        if args.command == "render-bst":
            return render_buildstream(args.target)
        return run_engine(args)
    except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired, RuntimeError) as error:
        print(f"nexe bridge: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
