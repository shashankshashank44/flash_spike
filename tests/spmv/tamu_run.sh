#!/bin/sh
rm tamu_log.txt
for f in ~/RISCV/spike/riscv-isa-sim/tests/spmv/tamu/*.mtx
do
    echo "Processing $f"
	echo "Sparse Vector" >> tamu_log.txt
    ./spV_tamu.sh $f 
	grep "Took" run.out >> tamu_log.txt
	grep "Passed!" run.out >> tamu_log.txt
	grep "Failed!" run.out >> tamu_log.txt
	echo "Helper" >> tamu_log.txt
    ./helper_tamu.sh $f 
	grep "Took" run.out >> tamu_log.txt
	grep "Passed!" run.out >> tamu_log.txt
	grep "Failed!" run.out >> tamu_log.txt
done
