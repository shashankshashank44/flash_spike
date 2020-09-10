#!/bin/sh
./gen_rand 2 $1 10 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 > denseScalar.txt
./gen_rand 2 $1 20 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 30 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 40 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 50 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 60 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 70 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 80 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
./gen_rand 2 $1 90 temp.h temp
./denseScalar.sh > log.txt ;  grep "Took" log.txt | cut -d ' ' -f 2 >> denseScalar.txt
