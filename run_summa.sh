#!/usr/bin/env bash
total=$(($1*$1))
echo "total:"${total}
mpirun --oversubscribe -np $total xterm -e "./summa $1 $2 $3 $4"
