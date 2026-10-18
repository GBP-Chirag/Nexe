#!/usr/bin/env bash

set -e

echo "🚀 Setting up Nexe (C++ Core)..."

# 1. Verify system requirements
if ! command -v cmake &> /dev/null; then
    echo "❌ Error: cmake is required but not installed." >&2
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "❌ Error: python3 is required but not installed." >&2
    exit 1
fi

# 2. Build the C++ Performance Core for the current AMD64/AArch64 machine.
echo "🔨 Compiling C++ DAG and Hashing engine..."
cmake -S . -B build
cmake --build build --parallel "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"

echo "✅ Nexe installation complete! Verify it with: ./build/nexe_engine build BUILD.sbl"
