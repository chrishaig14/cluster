#!/usr/bin/env bash
total=$(($1*$2))
echo "total:"${total}
mpirun --oversubscribe -np $total xterm -hold -e "./summa $1 $2 $3 $4 $5"
