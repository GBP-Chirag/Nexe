# Nexe / Postake

A high-performance, polyglot build system designed with a **"Dual-Mode" architecture**. For small local projects, it runs on a lightning-fast custom C/C++/Go core with zero overhead. When projects scale up, it seamlessly transpiles into industrial-grade enterprise tooling using Apache BuildStream, OpenDAL, and REAPI.

---

## 🏗️ Polyglot Architecture & Technology Stack

Nexe leverages the best tool for every layer of the build pipeline:

* **Go**: CLI orchestration and lightweight concurrent task dispatch.
* **C++**: Performance core handling Topological Sorting (Kahn's algorithm) and Content-Addressable Hashing.
* **C (Bison & Flex)**: High-performance lexical analysis and parsing for your custom `.sbl` Domain Specific Language.
* **Python**: Apache BuildStream integration and Jinja2 template generation for enterprise-scale configurations.
* **Rust**: Future-proof cloud storage and object caching via Apache OpenDAL.
* **Protobuf & gRPC**: Standardized Remote Execution API (REAPI) bridging for local-to-remote worker scaling.

---

## 📂 Project Structure

```text
nexe/
├── CMakeLists.txt                 # Builds the C++ core engine
├── Makefile                       # Top-level automation for polyglot compilation
├── go.mod / go.sum                # Go CLI & concurrent dispatcher dependencies
├── Cargo.toml                     # Rust OpenDAL storage module (Future)
├── pyproject.toml                 # Python orchestration & BuildStream configuration
├── proto/                         # Shared REAPI Protocol Buffer definitions
│   └── remote_execution.proto
├── dsl/                           # Bison & Flex Parser (C)
│   ├── lexer.l                    # Flex token rules for .sbl files
│   ├── parser.y                   # Bison grammar for MODULE.sbl & BUILD.sbl
│   └── ast.h                      # Abstract Syntax Tree definitions
├── core_cpp/                      # C++ Performance Core
│   ├── include/engine.hpp         # DAG and Hashing declarations
│   └── src/                       # DAG sorting & hashing logic
├── cli_go/                        # Go CLI & Concurrency Dispatcher
│   ├── main.go                    # Entry point for user commands
│   └── dispatcher.go              # Concurrent task runner
├── storage_rust/                  # OpenDAL Cloud Storage Bridge (Rust)
│   └── lib.rs                     # S3/GCS operations
├── orchestrator_py/               # Python BuildStream & Jinja2 Engine
│   ├── bridge.py                  # Translates parsed SBL to .bst (BuildStream)
│   └── templates/                 # Jinja2 templates for large-scale configs
├── scripts/
│   ├── bootstrap.sh               # Environment and dependency check script
│   └── install.sh                 # Compilation and setup automation
└── tests/
    └── integration_test.go        # End-to-end polyglot checks
