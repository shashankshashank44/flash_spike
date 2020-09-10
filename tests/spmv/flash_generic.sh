#!/bin/sh
rm a.out
echo "script arguments: $1-matrix_input_path, $2-storage(0-dense;1-CSR;2-BV;3-RL), $3-compute(0-dense,1-csr,2-bv,3-rl,4-hht_dense,5-hht_sparse)"

# Compile iteration #1
./build_helper_scalar_gen.sh 
# Run spike iteration #1
cd ../../build
./spike --dc=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 0 $1 $2 $3 $4 | grep "#define"  > ../tests/spmv/generated_macros.h
# Compile iteration #2
cd ../tests/spmv
./build_helper_scalar.sh
# Run spike iteration #2
cd ../../build
./spike --dc=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 ../../../riscv-pk/build/pk ../tests/spmv/a.out 0 0 $1 $2 $3 $4 | grep "#define"  > ../tests/spmv/generated_macros.h
# Compile iteration #3
cd ../tests/spmv
./build_helper_scalar.sh
/home/UNT/sa0659/RISCV/bin/riscv32-unknown-elf-objdump -D a.out > a.dump
# Run spike iteration #3
cd ../../build
./spike --dc=32768:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 $1 $2 $3 $4
#./spike --dc=512:4:32 --l2=4096:4:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 ../tests/spmv/temp_dense.dat 1 3 8
#./spike --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 ../tests/spmv/temp_dense.dat 1 3 8
cd ../tests/spmv
