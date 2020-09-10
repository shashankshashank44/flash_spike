#./sparseScalar.sh >& sparseS_4096.out
#./denseScalar.sh >& denseS_4096.out
#./sparseVector.sh >& sparseV_4096.out
#./denseVector.sh >& denseV_4096.out
#./helper.sh >& helperV_4096.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv-exp/tests/spmv/4096/temp_dense_0.dat >& HHT_expand_4k_1buf_0.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv-exp/tests/spmv/4096/temp_dense_0.dat >& HHT_expand_MCU_4k_1buf_0.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_10.dat >& HHT_expand_4k_1buf_10.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_10.dat >& HHT_expand_MCU_4k_1buf_10.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_20.dat >& HHT_expand_4k_1buf_20.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_20.dat >& HHT_expand_MCU_4k_1buf_20.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_30.dat >& HHT_expand_4k_1buf_30.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_30.dat >& HHT_expand_MCU_4k_1buf_30.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_40.dat >& HHT_expand_4k_1buf_40.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_40.dat >& HHT_expand_MCU_4k_1buf_40.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_50.dat >& HHT_expand_4k_1buf_50.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_50.dat >& HHT_expand_MCU_4k_1buf_50.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_60.dat >& HHT_expand_4k_1buf_60.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_60.dat >& HHT_expand_MCU_4k_1buf_60.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_70.dat >& HHT_expand_4k_1buf_70.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_70.dat >& HHT_expand_MCU_4k_1buf_70.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_80.dat >& HHT_expand_4k_1buf_80.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_80.dat >& HHT_expand_MCU_4k_1buf_80.out

./cHHT_sCSR_expanded_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_90.dat >& HHT_expand_4k_1buf_90.out
./cHHT_sCSR_expanded.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/temp_dense_90.dat >& HHT_expand_MCU_4k_1buf_90.out



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


