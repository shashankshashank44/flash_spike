#!/bin/sh
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-g++ -O3 -fpermissive -I. -c -DGENERATE_ONLY  generate.c
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-g++ -O3 -fpermissive -I. -DGENERATE_ONLY  denseMV.c generate.o
