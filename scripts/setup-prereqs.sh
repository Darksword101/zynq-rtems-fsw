#!/usr/bin/env bash
set -euo pipefail
sudo apt-get update
sudo apt-get install -y \
  build-essential g++ gdb git curl unzip pax bison flex texinfo \
  python3 python3-dev python3-pip python3-venv python-is-python3 \
  libncurses-dev zlib1g-dev libexpat1-dev libtinfo-dev \
  pkg-config ninja-build netcat-openbsd xxd \
  qemu-system-arm
qemu-system-arm --version
