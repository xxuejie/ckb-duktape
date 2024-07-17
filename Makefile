CLANG := $(shell scripts/find_clang)
LD := $(subst clang,ld.lld,$(CLANG))

MUSL := deps/musl
BUILTINS := deps/builtins

CFLAGS := -O2 -g --target=riscv64 -march=rv64imc_zba_zbb_zbc_zbs \
	-fdata-sections -ffunction-sections -fvisibility=hidden \
	-Wall -Werror -Wno-unused-but-set-variable \
	-nostdinc --sysroot $(MUSL)/release -isystem $(MUSL)/release/include \
	-Iduktape -Ic -Ischema -Ideps/ckb-c-stdlib -Ideps/ckb-c-stdlib/molecule

LIBS := build/duktape.o $(BUILTINS)/build/libcompiler-rt.a
LDFLAGS := build/duktape.o -static --gc-sections -s -L $(MUSL)/release/lib -L$(BUILTINS)/build -lc -lm -lgcc -lcompiler-rt

BINS := build/duktape build/repl build/dump_load build/native_dump_bytecode build/dump_load_nocleanup build/native_args_assembler

all: $(BINS)

build/native_dump_bytecode: c/native_dump_bytecode.c duktape/duktape.c
	gcc -Wall -Werror -Iduktape -O3 $^ -o $@ -lm

build/native_args_assembler: c/native_args_assembler.c
	gcc -Wall -Werror -Ischema -Ideps/ckb-c-stdlib/molecule -O3 $^ -o $@

build/duktape: build/entry.o $(LIBS)
	$(LD) $< -o $@ $(LDFLAGS)

build/dump_load: build/dump_load.o $(LIBS)
	$(LD) $< -o $@ $(LDFLAGS)

build/dump_load_nocleanup: build/dump_load_nocleanup.o $(LIBS)
	$(LD) $< -o $@ $(LDFLAGS)

build/repl: build/repl.o build/duktape.o $(LIBS)
	$(LD) $< -o $@ $(LDFLAGS)

build/%.o: c/%.c c/glue.h $(MUSL)/release/include/stddef.h
	$(CLANG) $(CFLAGS) $< -c -o $@

build/duktape.o: duktape/duktape.c $(MUSL)/release/include/stddef.h
	$(CLANG) $(CFLAGS) $< -c -o $@

$(BUILTINS)/build/libcompiler-rt.a:
	cd $(BUILTINS) && \
		make CC=$(CLANG) \
		  LD=$(subst clang,ld.lld,$(CLANG)) \
			OBJCOPY=$(subst clang,llvm-objcopy,$(CLANG)) \
			AR=$(subst clang,llvm-ar,$(CLANG)) \
			RANLIB=$(subst clang,llvm-ranlib,$(CLANG))

$(MUSL)/release/include/stddef.h:
	cd $(MUSL) && \
		CLANG=$(CLANG) ./ckb/build.sh

clean:
	rm -rf build/*.o $(BINS)
	cd $(BUILTINS) && make clean
	cd $(MUSL) && make clean && rm -rf release

dist: clean all

.PHONY: clean dist all
