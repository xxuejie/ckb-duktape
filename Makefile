TARGET := riscv64-unknown-elf
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
CFLAGS := -Os -DCKB_NO_MMU -D__riscv_soft_float -D__riscv_float_abi_soft
APP_CFLAGS := $(CFLAGS) -Iduktape -Ic -Ischema -Ideps/ckb-c-stdlib -Ideps/ckb-c-stdlib/molecule -Wall -Werror
# LDFLAGS := -lm -g
LDFLAGS := -lm -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
CURRENT_DIR := $(shell pwd)

all: build/duktape build/repl build/dump_load build/native_dump_bytecode build/dump_load_nocleanup

build/native_dump_bytecode: c/native_dump_bytecode.c duktape/duktape.c
	gcc -Wall -Werror -Iduktape -O3 $^ -o $@ -lm

build/duktape: build/entry.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/dump_load: build/dump_load.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/dump_load_nocleanup: build/dump_load_nocleanup.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/repl: build/repl.o build/duktape.o
	$(LD) $^ -o $@ $(LDFLAGS)

build/entry.o: c/entry.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/dump_load.o: c/dump_load.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/dump_load_nocleanup.o: c/dump_load_nocleanup.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/repl.o: c/repl.c c/glue.h
	$(CC) $(APP_CFLAGS) $< -c -o $@

build/duktape.o: duktape/duktape.c
	$(CC) $(APP_CFLAGS) $< -c -o $@

clean:
	rm -rf build/*.o build/duktape build/repl build/dump_load build/native_dump_bytecode build/dump_load_nocleanup

dist: clean all

.PHONY: clean dist all
