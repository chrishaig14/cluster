#!/usr/bin/env bash

# $4: m_proc
# $5: n_proc

./random_matrix $1 $2 a 10
./random_matrix $2 $3 b 10
time ./run_summa.sh $4 $5 a b result_1
time ./mult a b result_2
diff -s result_1 result_2
