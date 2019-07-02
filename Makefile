TARGET := riscv64-unknown-elf
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
CFLAGS := -Os -DCKB_NO_MMU -D__riscv_soft_float -D__riscv_float_abi_soft
APP_CFLAGS := $(CFLAGS) -Iduktape -Ic -Wall -Werror
# LDFLAGS := -lm -g
LDFLAGS := -lm -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
CURRENT_DIR := $(shell pwd)

all: build/duktape

build/duktape: build/entry.o build/duktape.o
	$(LD) build/entry.o build/duktape.o -o $@ $(LDFLAGS)

build/entry.o: c/entry.c
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/duktape.o: duktape/duktape.c
	$(CC) $(APP_CFLAGS) $< -c -o $@

clean:
	rm -rf build/*.o build/duktape

dist: clean all

.PHONY: clean dist all
