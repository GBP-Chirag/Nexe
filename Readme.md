<div align="left">
  ![Nexe Logo](./image.png)

Welcome to the official Nexe documentation wiki! This page provides a complete architectural breakdown, philosophy, roadmap, usage guides, and contribution guidelines for the Nexe build system.

# What is Nexe?

Nexe is a custom-built, minimalist, high-performance, polyglot build system engineered for total architectural ownership. It bridges the gap between small local compilation pipelines and enterprise-scale multi-language builds without relying on heavy Python interpreters, uncustomizable third-party frameworks, or rigid execution protocols.

<p>
    <img src="https://img.shields.io/badge/core-C++%20%2F%20C-blue" alt="Core C/C++">
    <img src="https://img.shields.io/badge/cli-%20Go-00ADD8" alt="Go CLI">
    <img src="https://img.shields.io/badge/engine-BuildKit%20%2B%20Dagger-orange" alt="Unified Engine">
    <img src="https://img.shields.io/badge/license-GPL-green" alt="License">
  </p>
</div>

### Nexe models codebases as a living network graph:

    Nodes (Dots): Directories and workspace folders.

    Edges: Files and relational dependencies connecting each node.

### The Polyglot Tech Stack

    C (core_c/): High-speed lexical analysis (Flex), parsing (Bison), AST generation, and raw cryptographic hashing via BLAKE3.

    C++ (core_cpp/): Performance-critical Directed Acyclic Graph (DAG) sorting, content-addressable storage (CAS), and local cache validation.

    Go (cmd/nexe/, pkg/): Concurrent task dispatcher, worker pool scheduler, process isolation, and universal package command delegation.

    Embedded BuildKit + Dagger: In-process container-native execution engine triggered when scaling to enterprise workloads.

    Config Formats: Starlark (.sbl scripts), Canonical JSON (nexe.workspace.json), and TOML (nexe.toml, NTAKE.toml).

# Why Was Nexe Built?

Existing build systems force developers into trade-offs:

    Traditional build tools (like Make, CMake, Ninja) excel at fast local execution but struggle with container-native isolation, distributed caching, and clean monorepo management across languages.

    Enterprise systems (like Bazel or legacy BuildStream architectures) offer reproducibility and scale, but often come with steep learning curves, massive interpreter overhead (Python/Java), or locked protocol structures.

### The Nexe Solution

Nexe was built to deliver uncompromised speed and total control:

    Native Speed First: Small to medium projects compile instantly using local C/C++ memory structures and memory-mapped file access.

    Dynamic Scaling Router: When a project crosses the 50,000 Lines of Code (LOC) threshold, Nexe automatically routes execution to an embedded BuildKit + Dagger control plane running inside the Go process memory.

    Universal Package Management: Integrates NTAKE.toml to unify dependencies across ecosystems (Python, Node, Rust, Go) under a single declarative command syntax.

    Full OS & Container Output: Native support for emitting standard object binaries, compressed archives (.tar, .gz), OCI container images, and full bootable OS sysroot images (.iso).

 # Roadmap & Future Plans

Nexe is built in parallel with a rigorous systems engineering curriculum (C, C++, Linux Kernel, CyberOps). Below is the multi-stage deployment path:
Phase 1: Local Polyglot Core (v0.1 — Current)

    [x] Establish Flex/Bison parsing flow and BLAKE3 C++ cryptographic hash validation.

    [x] Implement folder-as-node & file-as-edge C++ DAG graph engine.

    [x] Define Go CLI router and 50K LOC threshold scheduler.

    [x] Scaffold NTAKE.toml package delegation layer.

Phase 2: In-Process Engine & Universal Storage (v0.2)

    [ ] Direct embedded integration of moby/buildkit and dagger/dagger Go modules.

    [ ] Implement OpenDAL storage abstraction for seamless local disk, tarball, and S3 caching.

    [ ] High-density code compression pipeline (Vector sorting + Zstd/LZMA pass).

    [ ] Header/payload encryption using Argon2id + AES-256-GCM.

Phase 3: Advanced Storage & Networking (v0.3)

    [ ] Bidirectional gRPC-over-TCP multiplexed socket tunnel pipe for distributed nodes.

    [ ] Custom Multi-Version Concurrency Control (MVCC) build ledger for instant incremental rollbacks.

    [ ] Rule-based Starlark Query Optimizer for large workspace AST evaluations.

Phase 4: Enterprise Orchestration & Ecosystem (v1.0)

    [ ] Cadence/Temporal-style workflow backend orchestration.

    [ ] Full OS rootfs generation and automated .iso bootable image bundling via LLB exporters.

    [ ] CI/CD pipeline plugins and native IDE extensions.

# Configuration & Usage Examples
1. Defining Workspace Nodes (nexe.workspace.json)
JSON

{
  "workspace": "monorepo-core",
  "root_nodes": ["src/", "core_cpp/", "cmd/"],
  "cache_backend": "local",
  "loc_threshold": 50000
}

2. Defining Build Scripts (MODULE.sbl)
Code snippet

target "nexe_core" {
    sources = ["core_cpp/engine.cpp", "core_cpp/dag.cpp"]
    flags   = ["-O3", "-std=c++20"]
    output  = "bin/nexe-core"
}

3. Universal Package Manifest (NTAKE.toml)
Ini, TOML

[project]
name = "nexe-service"
version = "0.1.0"

[dependencies]
python = ["requests==2.31.0"]
node   = ["express@4.18.2"]
rust   = ["tokio = { version = \"1.0\" }"]

# 🤝 Contribution Guidelines

We welcome contributions from systems engineers, language enthusiasts, and open-source developers!
Standard Workflow

    Fork the Repository: Create your own copy of GBP-Chirag/Nexe.

    Create a Feature Branch:
    Bash

    git checkout -b feat/dag-optimizer

    Follow Code Rules:

        C/C++ (core_c/, core_cpp/): Follow strict RAII principles, avoid unmanaged raw pointer allocations, and ensure no memory leaks exist on parse failures.

        Go (pkg/, cmd/): Ensure thread safety across goroutines and keep context cancellation clean.

    Run Tests: Ensure all existing C/C++ unit tests and Go worker checks pass:
    Bash

    make test

    Open a Pull Request: Submit a descriptive PR linking to any relevant issue or architectural spec.

Questions or Suggestions?

Feel free to open an issue or reach out via discussions on the Nexe GitHub Repository!