Test program for AES

Requires that RISCV toolchain is been built from https://github.com/riscv-collab/riscv-gnu-toolchain.

Use the flags "--enable-multilib" when running configure. This is to be able to build 32 bit RISCV, with "-march=rv32ima -mabi=ilp32".