#!/bin/sh
rm a.out
# Compile iteration #1
riscv32-unknown-elf-g++ -I. $1
# Run spike iteration #1
cd ../build
./spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../pk/riscv-pk/build/pk ../tests/a.out | grep "#define"  > ../tests/generated_macros.h
# Compile iteration #2
cd ../tests
riscv32-unknown-elf-g++ -I. $1
# Run spike iteration #2
cd ../build
./spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../pk/riscv-pk/build/pk ../tests/a.out | grep "#define"  > ../tests/generated_macros.h
# Compile iteration #3
cd ../tests
riscv32-unknown-elf-g++ -I. $1
# Run spike iteration #3
cd ../build
./spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../pk/riscv-pk/build/pk ../tests/a.out 
