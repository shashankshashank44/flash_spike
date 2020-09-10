#./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt  ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/2048/temp_dense_90.dat 1 1 1 >& pref/CSR_2048/90_2048_$1-pref &

#./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt  ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/2048/temp_dense_50.dat 1 1 1 >& pref/CSR_2048/50_2048_$1-pref &

#./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt  ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/2048/temp_dense_10.dat 1 1 1 >& pref/CSR_2048/10_2048_$1-pref &


~/RISCV/spike/riscv-all-versions-prefetch/build/./spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=/home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/timings.txt /home/UNT/sa0659/RISCV/riscv-pk/build/pk /home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/2048/temp_dense_90.dat 1 1 8 >& pref/CSR_2048/90_2048_$1-pref &

~/RISCV/spike/riscv-all-versions-prefetch/build/./spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=/home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/timings.txt /home/UNT/sa0659/RISCV/riscv-pk/build/pk /home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/2048/temp_dense_50.dat 1 1 8 >& pref/CSR_2048/50_2048_$1-pref &

~/RISCV/spike/riscv-all-versions-prefetch/build/./spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=/home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/timings.txt /home/UNT/sa0659/RISCV/riscv-pk/build/pk /home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-all-versions-prefetch/tests/spmv/2048/temp_dense_10.dat 1 1 8 >& pref/CSR_2048/10_2048_$1-pref &
