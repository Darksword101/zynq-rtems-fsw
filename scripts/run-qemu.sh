#!/usr/bin/env bash
set -euo pipefail
EXE="${1:-build/fsw.exe}"
PORT="${TLM_PORT:-5555}"
echo "TLM/CMD link: tcp://127.0.0.1:${PORT}   console: this terminal   quit: Ctrl-A x"
exec qemu-system-arm -M xilinx-zynq-a9 -m 256M -no-reboot -nographic \
  -serial "tcp:127.0.0.1:${PORT},server=on,wait=off" \
  -serial mon:stdio \
  -kernel "$EXE"
