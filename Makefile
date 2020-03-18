TARGET := riscv64-unknown-elf
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
CFLAGS := -Os -DCKB_NO_MMU -D__riscv_soft_float -D__riscv_float_abi_soft
APP_CFLAGS := $(CFLAGS) -Iduktape -Ic -Ideps/ckb-c-stdlib -Ideps/molecule -Wall -Werror
# LDFLAGS := -lm -g
LDFLAGS := -lm -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
CURRENT_DIR := $(shell pwd)

all: build/duktape build/repl build/load0 build/repl0 build/dump_load0

build/duktape: build/entry.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/dump_load0: build/dump_load0.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/load0: build/load0.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/repl: build/repl.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/repl0: build/repl0.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/entry.o: c/entry.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/dump_load0.o: c/dump_load0.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/load0.o: c/load0.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/repl.o: c/repl.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/repl0.o: c/repl0.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/duktape.o: duktape/duktape.c
	$(CC) $(APP_CFLAGS) $< -c -o $@

clean:
	rm -rf build/*.o build/duktape build/repl build/load0 build/repl0

dist: clean all

.PHONY: clean dist all
