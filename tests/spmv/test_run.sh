
rm 512_results/*
cd 512/
for file in temp_dense_10.dat 
#for file in temp_dense_10.dat temp_dense_20.dat temp_dense_30.dat temp_dense_40.dat temp_dense_50.dat temp_dense_60.dat temp_dense_70.dat temp_dense_80.dat temp_dense_90.dat 
#for file in denseNet_weights.txt mobileNetV2_weights.txt mobileNet_weights.txt resnetV2_weights.txt resnet_weights.txt vgg16_weights.txt vgg19_weights.txt 
do
echo $file
cd ..



./cHHT_sCSR_noexpand_HP.sh /home/UNT/sa0659/RISCV/spike/riscv-flash/tests/spmv/512/$file >& 512_results/HHT_CSR_vector8-$file.out 
wait
./../../build/spike --dc=32768:1:32 --isa=RV32IMAFDCV --varch=v256:e32:s256 --timing=timings.txt ../../../../riscv-pk/build/pk a.out 1 0 /home/UNT/sa0659/RISCV/spike/riscv-flash/tests/spmv/512/$file 1 1 8 >& 512_results/CSR_vector8-$file.out &                                                                                                                                                                                                                                       
wait
cd 512
done
