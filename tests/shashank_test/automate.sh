#!/bin/sh
rm a.out
# Compile iteration #1
riscv32-unknown-elf-g++ -I. -std=c++11 $1
# Run spike iteration #1
../../build/spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../../pk/riscv-pk/build/pk ./a.out ./inputs.out | grep "#define"  > ./generated_macros.h
# Compile iteration #2
riscv32-unknown-elf-g++ -I. -std=c++11 $1
# Run spike iteration #2
../../build/spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../../pk/riscv-pk/build/pk ./a.out ./inputs.out | grep "#define"  > ./generated_macros.h
# Compile iteration #3
riscv32-unknown-elf-g++ -I. -std=c++11 $1
riscv32-unknown-elf-objdump -D ./a.out > ./a.dump
# Run spike iteration #3
../../build/spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../../pk/riscv-pk/build/pk ./a.out ./inputs.out
