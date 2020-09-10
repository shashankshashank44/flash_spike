#!/bin/sh
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-g++ -O3 -fpermissive -I. -c generate.c
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-g++ -O3 -fpermissive -I. denseMV.c generate.o
