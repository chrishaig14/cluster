#!/usr/bin/env bash
total=$(($1*$1))
mpirun --oversubscribe -np $total xterm -e "./summa $1 $2 $3 $4"
