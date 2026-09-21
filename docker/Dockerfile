FROM ubuntu:24.04 AS toolchain
ENV DEBIAN_FRONTEND=noninteractive RTEMS_PREFIX=/opt/rtems/6
RUN apt-get update && apt-get install -y build-essential g++ gdb git curl unzip pax bison flex texinfo \
    python3 python3-dev python-is-python3 libncurses-dev zlib1g-dev libexpat1-dev libtinfo-dev pkg-config ninja-build \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
RUN curl -L https://ftp.rtems.org/pub/rtems/releases/6/6.1/sources/rtems-source-builder-6.1.tar.xz | tar xJf - \
 && cd rtems-source-builder-6.1/rtems && ../source-builder/sb-set-builder --prefix=$RTEMS_PREFIX 6/rtems-arm \
 && rm -rf /src/rtems-source-builder-6.1
RUN curl -L https://ftp.rtems.org/pub/rtems/releases/6/6.1/sources/rtems-6.1.tar.xz | tar xJf - \
 && cd rtems-6.1 && printf '[arm/xilinx_zynq_a9_qemu]\nRTEMS_POSIX_API = True\n' > config.ini \
 && export PATH=$RTEMS_PREFIX/bin:$PATH && ./waf configure --prefix=$RTEMS_PREFIX && ./waf && ./waf install \
 && rm -rf /src/rtems-6.1

FROM ubuntu:24.04
ENV RTEMS_PREFIX=/opt/rtems/6 PATH=/opt/rtems/6/bin:$PATH PKG_CONFIG_PATH=/opt/rtems/6/lib/pkgconfig
COPY --from=toolchain /opt/rtems/6 /opt/rtems/6
RUN apt-get update && apt-get install -y --no-install-recommends make gcc pkg-config qemu-system-arm python3 python3-pip \
    && pip3 install --break-system-packages pytest pytest-timeout && rm -rf /var/lib/apt/lists/*
WORKDIR /work
