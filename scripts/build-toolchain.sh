#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/env.sh"
SRC="$HOME/rtems/src"
mkdir -p "$SRC" && cd "$SRC"

if [ ! -d rsb ]; then
  curl -L https://ftp.rtems.org/pub/rtems/releases/6/6.1/sources/rtems-source-builder-6.1.tar.xz | tar xJf -
  mv rtems-source-builder-6.1 rsb
fi

cd rsb/rtems
../source-builder/sb-check                 # verifies host prerequisites
../source-builder/sb-set-builder --prefix="$RTEMS_PREFIX" 6/rtems-arm   2>&1 | tee "$SRC/toolchain-build.log"

"$RTEMS_PREFIX/bin/arm-rtems6-gcc" --version
