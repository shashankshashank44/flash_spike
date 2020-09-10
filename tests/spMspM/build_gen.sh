#!/bin/sh
riscv32-unknown-elf-g++ -O3 -fpermissive -I. -c -DGENERATE_ONLY  generate.c
riscv32-unknown-elf-g++ -O3 -fpermissive -I. -DGENERATE_ONLY  denseMV.c generate.o
