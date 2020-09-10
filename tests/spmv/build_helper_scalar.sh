#!/bin/sh
rm a.out
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-g++ -O3 -fpermissive -I. -march=rv32imc -c -DRISCV_BUILD generate.c
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-g++ -O3 -fpermissive -I. -march=rv32imc -DRISCV_BUILD denseMV_scalar.c generate.o
