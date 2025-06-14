#!/bin/sh
# setup.sh - Install dependencies for building and testing this project.
# This script installs clang and related tools so that the sources can be
# compiled as C++23 and analyzed with clang-tidy and clang-format.

set -euo pipefail

# Update package lists.
sudo apt-get update

# Install build utilities, analysis tools and documentation generators.
sudo apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    nasm \
    clang \
    clang-tidy \
    clang-format \
    clang-tools \
    clangd \
    lld \
    llvm \
    llvm-dev \
    libclang-dev \
    libclang-cpp-dev \
    cppcheck \
    valgrind \
    lcov \
    strace \
    gdb \
    rustc \
    cargo \
    rustfmt \
    g++ \
    afl++ \
    ninja-build \
    doxygen \
    graphviz \
    python3-sphinx \
    python3-breathe \
    python3-sphinx-rtd-theme \
    python3-pip \
    libsodium-dev \
    libsodium23 \
    qemu-system-x86 \
    qemu-utils \
    qemu-user \
    tmux \
    cloc

# Ensure ack is installed for convenient searching
if ! command -v ack >/dev/null 2>&1; then
    # Install the ACK compiler suite along with development headers
    sudo apt-get install -y --no-install-recommends \
        ack \
        ack-grep
fi

# Install optional ack helpers for Python and Node
# ack helpers are optional; skip if pip or npm are restricted

# Attempt to install libfuzzer development package if available.
sudo apt-get install -y --no-install-recommends libfuzzer-dev || true

