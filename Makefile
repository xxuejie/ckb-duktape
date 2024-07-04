CLANG := $(shell scripts/find_clang)
LD := $(subst clang,ld.lld,$(CLANG))

CFLAGS := -O3 -g --target=riscv64 -march=rv64imc_zba_zbb_zbc_zbs \
	-fdata-sections -ffunction-sections -fvisibility=hidden \
	-Wall -Werror -Wno-unused-but-set-variable -ferror-limit=200 \
	-DCKB_C_STDLIB_PRINTF -DCKB_MALLOC_DECLARATION_ONLY \
	-DCKB_DECLARATION_ONLY -DCKB_STDLIB_NO_SYSCALL_IMPL \
	-nostdinc -nostdlib -Iaugmented-libc -Ideps/ckb-c-stdlib/libc \
	-Iduktape -Ic -Ideps/ckb-c-stdlib -Ideps/molecule
# LDFLAGS := -lm -g
LDFLAGS := -static --gc-sections -s --error-limit=200

AUGMENTED_LIBC_SOURCES := $(shell find augmented-libc/impls -name '*.c')
AUGMENTED_LIBC_OBJECTS := $(addprefix build/,$(AUGMENTED_LIBC_SOURCES:%.c=%.o))

LIBS := build/duktape.o deps/compiler-builtins/build/libcompiler-rt.a $(AUGMENTED_LIBC_OBJECTS) build/libc.o

all: build/duktape build/repl build/load0 build/repl0 build/dump_load0 build/dump_bytecode build/dump_load0_nocleanup

build/dump_bytecode: c/dump_bytecode.c duktape/duktape.c
	gcc -Wall -Werror -Iduktape -O3 $^ -o $@ -lm

build/duktape: build/entry.o $(LIBS)
	$(LD) $^ -o $@ $(LDFLAGS)

build/dump_load0: build/dump_load0.o $(LIBS)
	$(LD) $^ -o $@ $(LDFLAGS)

build/dump_load0_nocleanup: build/dump_load0_nocleanup.o $(LIBS)
	$(LD) $^ -o $@ $(LDFLAGS)

build/load0: build/load0.o $(LIBS)
	$(LD) $^ -o $@ $(LDFLAGS)

build/repl: build/repl.o $(LIBS)
	$(LD) $^ -o $@ $(LDFLAGS)

build/repl0: build/repl0.o $(LIBS)
	$(LD) $^ -o $@ $(LDFLAGS)

build/entry.o: c/entry.c c/glue.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/dump_load0.o: c/dump_load0.c c/glue.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/dump_load0_nocleanup.o: c/dump_load0_nocleanup.c c/glue.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/load0.o: c/load0.c c/glue.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/repl.o: c/repl.c c/glue.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/repl0.o: c/repl0.c c/glue.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/duktape.o: duktape/duktape.c
	$(CLANG) $(CFLAGS) $< -c -o $@

build/augmented-libc/impls/%.o: augmented-libc/impls/%.c
	mkdir -p build/augmented-libc/impls
	$(CLANG) $(CFLAGS) $< -c -o $@	

build/libc.o: c/libc.c
	$(CLANG) $(CFLAGS) $< -c -o $@

deps/compiler-builtins/build/libcompiler-rt.a:
	cd deps/compiler-builtins && \
		make CC=$(CLANG) \
		  LD=$(subst clang,ld.lld,$(CLANG)) \
			OBJCOPY=$(subst clang,llvm-objcopy,$(CLANG)) \
			AR=$(subst clang,llvm-ar,$(CLANG)) \
			RANLIB=$(subst clang,llvm-ranlib,$(CLANG))

clean:
	rm -rf build/*.o build/duktape build/repl build/load0 build/repl0 build/dump_load0 build/dump_bytecode build/dump_load0_nocleanup build/augmented-libc
	cd deps/compiler-builtins && make clean

dist: clean all

.PHONY: clean dist all
