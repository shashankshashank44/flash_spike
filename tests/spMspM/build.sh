#!/bin/sh
riscv32-unknown-elf-g++ -O3 -fpermissive -I. -c generate.c
riscv32-unknown-elf-g++ -O3 -fpermissive -I. denseMV.c generate.o
