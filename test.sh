#!/usr/bin/env bash

./random_matrix $1 $2 a 10
./random_matrix $2 $3 b 10
./run_summa.sh 2 2 a.bin b.bin result_1
./mult a.bin b.bin result_2
diff -s result_1 result_2
