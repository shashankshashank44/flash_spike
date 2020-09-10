./sparseScalar.sh >& sparseS_4096.out
./denseScalar.sh >& denseS_4096.out
./sparseVector.sh >& sparseV_4096.out
./denseVector.sh >& denseV_4096.out
#./helper.sh >& helperV_4096.out

#cd ../../build
#./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 1 0 >& sparseS.out
#cd ../tests/spmv

#cd ../../build
#./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 1 1 >& sparseV.out
#cd ../tests/spmv

#cd ../../build
#./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 0 0  >& denseS.out
#cd ../tests/spmv

#cd ../../build
#./spike --dc=8192:1:32 --l2=32768:4:32 --isa=RV32IMCV --varch=v256:e32:s256 --timing=../tests/spmv/timings.txt ../../../riscv-pk/build/pk ../tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-spmv/tests/spmv/temp_dense.dat 0 1 >& denseV.out
#cd ../tests/spmv


