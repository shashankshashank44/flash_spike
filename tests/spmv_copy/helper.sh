#!/bin/sh
rm a.out
# Compile iteration #1
./build_gen.sh 
# Run spike iteration #1
cd ../../build
#make
./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 1 2 | grep "#define"  > ../tests/spmv/generated_macros.h
# Compile iteration #2
cd ../tests/spmv
./build.sh
# Run spike iteration #2
cd ../../build
./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 1 2 | grep "#define"  > ../tests/spmv/generated_macros.h
# Compile iteration #3
cd ../tests/spmv
./build.sh
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-objdump -D a.out > a.dump
# Run spike iteration #3
cd ../../build
./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 1 2  
cd ../tests/spmv
