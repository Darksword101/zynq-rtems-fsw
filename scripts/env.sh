export RTEMS_PREFIX="$HOME/rtems/6"
export RTEMS_BSP="xilinx_zynq_a9_qemu"
export RTEMS_ARCH="arm-rtems6"
export PATH="$RTEMS_PREFIX/bin:$PATH"          # prepend, never overwrite the system PATH
export PKG_CONFIG_PATH="$RTEMS_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
