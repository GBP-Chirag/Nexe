<div align="center">
  <img src="assets/nexe-logo.png" alt="Nexe Logo" width="280" />
  
 
# Nexe 

Nexe is a lightweight, declarative local build system. Its C++17 engine builds a Directed Acyclic Graph (DAG), fingerprints task inputs with BLAKE3, and skips a task when its inputs and command have not changed. The Go CLI uses Cobra and invokes the engine through a small Python bridge.

## Features (V1)
- **SBL Configuration**: A concise `BUILD.sbl` language for variables and build nodes.
- **TOML Compatibility**: Existing `build.toml` files remain supported while SBL becomes the default.
- **DAG Engine**: Resolves task order and reports circular or missing dependencies.
- **BLAKE3 Hashing**: Stores content fingerprints in `.nexe/state/` only after a task succeeds.
- **Incremental Builds**: A task is skipped when its command and declared inputs match its saved fingerprint and its outputs still exist.

## Example `BUILD.sbl`

```sbl
toolchain host_cpp {
    # Linux: g++, macOS: clang++, Windows Developer PowerShell: cl
    # Keep the rule unchanged and replace only this provider as needed.
    flavor = "gnu"
    cc = "gcc"
    cxx = "g++"
    cflags = []
    cxxflags = ["-std=c++17", "-Icore_cpp/include"]
    ldflags = []
}

cxx_binary build_nexe {
    toolchain "host_cpp"
    source "core_cpp/src/main.cpp"
    source "core_cpp/src/dag.cpp"
    source "core_cpp/src/hasher.cpp"
    target "build/nexe"
}
```

`cc_binary` and `cxx_binary` are built-in rules. They receive compiler names, flags, and linker flags from a named `toolchain` block; the same rule works with a GNU/Clang toolchain (`flavor = "gnu"`) or an MSVC one (`flavor = "msvc"`, `cc = "cl"`, `cxx = "cl"`). Compiler names are resolved from `PATH`, so build files do not need hardcoded machine-specific paths. Use `node` with `run` only for custom commands.

## Getting Started

1. Build the engine:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```
2. Run an SBL build file directly:
   ```bash
   ./build/nexe_engine build BUILD.sbl
   ```
3. Or install Go, then use the Cobra CLI:
   ```bash
   cd Nexe_CLI
   go run . init ../example
   go run . build ../example/BUILD.sbl
   ```

The bridge builds the C++ engine automatically when no `--engine` path or `NEXE_ENGINE` environment variable is supplied.

## BuildStream templates

`_Nexe/Orchestration/templates/buildstream/` is a BuildStream project template, not an Nexe build file. Its Starlark entrypoint (`render.star`) builds dictionaries and lists for elements and emits deterministic, static `.bst` YAML into `elements/`. It does not run shell commands or access the filesystem itself.

The checked-in `elements/hello.bst` is the current rendered output. Regenerate it after changing `render.star` with a Canonical Starlark interpreter:

```bash
go install github.com/canonical/starlark/cmd/starlark@latest
nexe render-bst _Nexe/Orchestration/templates/buildstream
```

Nexe validates every emitted path before writing and rejects output outside the template's `elements/` directory.

## Platforms and architectures

Nexe runs on Linux, macOS, and Windows for native AMD64 (`x86_64`) and AArch64 (`arm64`) toolchains. Unix task commands run through the system shell; Windows task commands run through PowerShell without requiring `cmd.exe` command quoting.

On Windows, build from PowerShell:

```powershell
.\install.ps1 -Architecture amd64
# Or use -Architecture aarch64 from an ARM64 Visual Studio toolchain.
```

For Linux cross-compilation, install the relevant GNU cross-compiler and pass a toolchain file:

```bash
cmake -S . -B build-aarch64 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-aarch64.cmake \
  -DNEXE_TARGET_ARCH=aarch64
cmake --build build-aarch64
```

## License
This project is licensed under the GNU General Public License v3.0 (GPLv3). See the [LICENSE](LICENSE) file for details.
