TARGET := riscv64-unknown-elf
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
CFLAGS := -Os -DCKB_NO_MMU -D__riscv_soft_float -D__riscv_float_abi_soft -Iduktape -Ic
# LDFLAGS := -lm -g
LDFLAGS := -lm -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
NEWLIB_LIB := build/newlib/$(TARGET)/lib/libc.a
CURRENT_DIR := $(shell pwd)

all: build/duktape

build/duktape: c/entry.c duktape/duktape.c $(NEWLIB_LIB)
	NEWLIB=build/newlib/$(TARGET) $(CC) -specs newlib-gcc.specs $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(NEWLIB_LIB):
	mkdir -p build/build-newlib && \
		cd build/build-newlib && \
		../../deps/newlib/configure --target=$(TARGET) --prefix=$(CURRENT_DIR)/build/newlib --enable-newlib-io-long-double --enable-newlib-io-long-long --enable-newlib-io-c99-formats CFLAGS_FOR_TARGET="$(CFLAGS)" && \
		make && \
		make install

clean-newlib:
	rm -rf build/newlib build/build-newlib

clean: clean-newlib

dist: clean all

.PHONY: clean clean-newlib dist all
