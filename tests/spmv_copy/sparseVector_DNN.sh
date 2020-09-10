#!/bin/sh
rm a.out
# Compile iteration #1
./build_gen.sh 
# Run spike iteration #1
cd ../../build
./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv_copy/a.out 0 3 $1 1 1 | grep "#define"  > ../tests/spmv_copy/generated_macros.h
# Compile iteration #2
cd ../tests/spmv_copy
./build.sh
# Run spike iteration #2
cd ../../build
./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv_copy/a.out 0 3 $1 1 1 | grep "#define"  > ../tests/spmv_copy/generated_macros.h
# Compile iteration #3
cd ../tests/spmv_copy
./build.sh
riscv32-unknown-elf-objdump -D a.out > a.dump
# Run spike iteration #3
cd ../../build
./spike  --dc=32768:4:32 --l2=131072:8:32 --timing=../tests/spmv_copy/timings.txt --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv_copy/a.out 1 3 $1 1 1  
cd ../tests/spmv_copy
