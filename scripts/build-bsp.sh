#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
SRC="$HOME/rtems/src"; cd "$SRC"

if [ ! -d rtems-6.1 ]; then
  curl -L https://ftp.rtems.org/pub/rtems/releases/6/6.1/sources/rtems-6.1.tar.xz | tar xJf -
fi
cd rtems-6.1

cat > config.ini <<EOF
[arm/xilinx_zynq_a9_qemu]
BUILD_SAMPLES = True
RTEMS_POSIX_API = True
RTEMS_DEBUG = False
EOF

./waf configure --prefix="$RTEMS_PREFIX"
./waf -j"$(nproc)"
./waf install

ls "$RTEMS_PREFIX/lib/pkgconfig/"        # expect arm-rtems6-xilinx_zynq_a9_qemu.pc
