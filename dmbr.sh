#!/usr/bin/env bash


# $1: File matrix A
# $2: File matrix B
# $3: N_PROC

cd DMBR/
make
cd ..
time mpirun --hostfile DMBR/host_file -np $3 DMBR/dmbr $1 $2
diff -s result_DMBR result_2