# Método por descomposición de matriz por columnas

Compilar con 

`mpicc -o main main.c matrix.c DMBR.c`

Ejecutar con

`mpirun -np <nro procesos> main`

Ejecutar con hostfile

`mpirun --hostfile host_file -np 4 main`

mpirun --hostfile host_file -np 4 "./main a b"