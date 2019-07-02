# ckb-duktape

Duktape build for CKB environment

# Build note

Since CKB VM doesn't have MMU available, a different solution to sbrk() should be leveraged to make sure a C script running in CKB VM can use malloc(). We have a patched [libc](https://github.com/nervosnetwork/riscv-newlib) serving as a PoC showing how this can be done but you are more than welcome to practice a different solution.

This means the easiest way to build this repository, is to use the GNU toolchain provided in this [docker image](https://hub.docker.com/r/nervos/ckb-riscv-gnu-toolchain). We have packed the GNU toolchain together with our modified libc, so everything should work out of the box. Notice that CKB VM doesn't require any modifications to gcc itself, so you should also be able to just use upstream gcc, and customize the libc used in linking phase.
