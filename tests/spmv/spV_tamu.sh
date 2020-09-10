#!/bin/sh
rm a.out
# Compile iteration #1
./build_gen.sh 
# Run spike iteration #1
cd ../../build
./spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 2 $1 1 1 | grep "#define"  > ../tests/spmv/generated_macros.h
# Compile iteration #2
cd ../tests/spmv
./build.sh
# Run spike iteration #2
cd ../../build
./spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 2 $1 1 1 | grep "#define"  > ../tests/spmv/generated_macros.h
#./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 2 $1 1 1 | grep "#define"  > ../tests/spmv/generated_macros.h
# Compile iteration #3
cd ../tests/spmv
./build.sh
../../../bin/riscv32-unknown-elf-objdump -D ../tests/spmv/a.out > ../tests/spmv/a.dump
# Run spike iteration #3
cd ../../build
#./spike --isa=RV32IMCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 2 $1 1 1 > ../tests/spmv/run.out 
./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 2 $1 1 1 > ../tests/spmv/run.out 
cd ../tests/spmv
