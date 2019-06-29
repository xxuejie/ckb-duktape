TARGET := riscv64-unknown-elf
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
CFLAGS := -Os -DCKB_NO_MMU -D__riscv_soft_float -D__riscv_float_abi_soft -Iduktape -Ic -Wall -Werror
# LDFLAGS := -lm -g
LDFLAGS := -lm -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
NEWLIB_LIB := build/newlib/$(TARGET)/lib/libc.a
CURRENT_DIR := $(shell pwd)

all: build/duktape

build/duktape: build/entry.o build/duktape.o $(NEWLIB_LIB)
	NEWLIB=build/newlib/$(TARGET) $(LD) -specs newlib-gcc.specs build/entry.o build/duktape.o -o $@ $(LDFLAGS)

build/entry.o: c/entry.c $(NEWLIB_LIB)
	NEWLIB=build/newlib/$(TARGET) $(CC) -specs newlib-gcc.specs $(CFLAGS) $< -c -o $@

build/duktape.o: duktape/duktape.c $(NEWLIB_LIB)
	NEWLIB=build/newlib/$(TARGET) $(CC) -specs newlib-gcc.specs $(CFLAGS) $< -c -o $@

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
