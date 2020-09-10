
rm 4096_new_results/*
cd 4096/
#for file in temp_dense_70.dat temp_dense_80.dat temp_dense_90.dat 
for file in temp_dense_10.dat temp_dense_20.dat temp_dense_30.dat temp_dense_40.dat temp_dense_50.dat temp_dense_60.dat temp_dense_70.dat temp_dense_80.dat temp_dense_90.dat 
#for file in denseNet_weights.txt mobileNetV2_weights.txt mobileNet_weights.txt resnetV2_weights.txt resnet_weights.txt vgg16_weights.txt vgg19_weights.txt 
do
echo $file
cd ..



./HHT_HP_generic.sh /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/$file 1 4 8 >& 4096_new_results/Exp_CSR-$file.out 
wait
#./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/$file 2 4 8 >& 4096_new_results/Exp_BV-$file.out &                                                                                                                                                                                                                                       
#./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/$file 3 4 8 >& 4096_new_results/Exp_RL-$file.out &                                                                                                                                                                                                                                       
./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/$file 1 5 8 >& 4096_new_results/HHT_CSR-$file.out &                                                                                                                                                                                                                                       
./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/$file 0 0 8 >& 4096_new_results/software_dense-$file.out &                                                                                                                                                                                                                                       
./../../build/spike --dc=32768:4:32 --l2=131072:8:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-isa-sim-memory-spmv/tests/spmv/4096/$file 1 1 8 >& 4096_new_results/software_CSR-$file.out &                                                                                                                                                                                                                                       

wait
cd 4096
done
