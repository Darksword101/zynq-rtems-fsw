# ── toolchain ────────────────────────────────────────────────────────────────
RTEMS_PREFIX ?= $(HOME)/rtems/6
RTEMS_ARCH   ?= arm-rtems6
RTEMS_BSP    ?= xilinx_zynq_a9_qemu
PKG           = $(RTEMS_ARCH)-$(RTEMS_BSP)
export PKG_CONFIG_PATH := $(RTEMS_PREFIX)/lib/pkgconfig

CC      = $(RTEMS_PREFIX)/bin/$(RTEMS_ARCH)-gcc
SIZE    = $(RTEMS_PREFIX)/bin/$(RTEMS_ARCH)-size
GDB     = $(RTEMS_PREFIX)/bin/$(RTEMS_ARCH)-gdb
HOSTCC  = gcc

INCS    = -Ifsw -Ifsw/config -Ifsw/platform -Ifsw/services -Ifsw/apps -Ifsw/demos
WARN    = -Wall -Wextra -Werror -Wshadow -Wundef -std=gnu11

TGT_CFLAGS  = $(shell pkg-config --cflags $(PKG)) -O2 -g -ffunction-sections -fdata-sections $(WARN) $(INCS)
TGT_LDFLAGS = $(shell pkg-config --libs $(PKG)) -Wl,--gc-sections -Wl,-Map=build/fsw.map -lm
ifdef DEMO
TGT_CFLAGS += -DFSW_RUN_PI_DEMO          # make DEMO=1 → runs pi_demo before starting apps
endif

# ── sources ──────────────────────────────────────────────────────────────────
FSW_SRC := $(wildcard fsw/*.c fsw/platform/*.c fsw/services/*.c fsw/apps/*.c fsw/demos/*.c)
FSW_OBJ := $(patsubst %.c,build/target/%.o,$(FSW_SRC))

PORTABLE_SRC := fsw/services/crc16.c fsw/services/ccsds.c fsw/services/fdir_rules.c fsw/services/sim_sensors.c
HOST_TESTS   := $(patsubst tests/host/%.c,build/host/%,$(wildcard tests/host/test_*.c))
HOST_CFLAGS  = -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(WARN) $(INCS) -Itests/host -DFSW_HOST_BUILD

# ── targets ──────────────────────────────────────────────────────────────────
.PHONY: all target host-test run debug sil size clean
all: target

target: build/fsw.exe size

build/fsw.exe: $(FSW_OBJ)
	$(CC) $(TGT_CFLAGS) $^ -o $@ $(TGT_LDFLAGS)

build/target/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(TGT_CFLAGS) -MMD -MP -c $< -o $@

size: build/fsw.exe
	$(SIZE) $<

host-test: $(HOST_TESTS)
	@for t in $^; do echo "── $$t"; $$t || exit 1; done

build/host/%: tests/host/%.c $(PORTABLE_SRC)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOST_CFLAGS) $^ -o $@ -lm

run: build/fsw.exe
	./scripts/run-qemu.sh $<

debug: build/fsw.exe
	./scripts/debug-qemu.sh $<

sil: build/fsw.exe
	python3 -m pytest tests/sil -v

clean:
	rm -rf build

-include $(FSW_OBJ:.o=.d)
