#!/usr/bin/env bash
set -e

echo "🚀 Setting up Postake V1 (Python + C++ Core)..."

# 1. Verify system requirements
if ! command -v cmake &> /dev/null; then
    echo "❌ Error: cmake is required but not installed." >&2
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "❌ Error: python3 is required but not installed." >&2
    exit 1
fi

# 2. Build the C++ Performance Core
echo "🔨 Compiling C++ DAG and Hashing engine..."
mkdir -p build
cd build
cmake ..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
cd ..

# 3. Install the Python CLI wrapper
echo "📦 Installing Python CLI package..."
python3 -m pip install --upgrade pip
python3 -m pip install -e .

echo "✅ Postake installation complete! Verify it works by running: postake --help"