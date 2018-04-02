# TP Final: Cluster MPI - Sistemas Distribuidos - FIUBA

Tutorial de MPI:

http://mpitutorial.com/tutorials/

## Instalar MPI:

Descargar OpenMPI: https://www.open-mpi.org/software/ompi/v3.0/downloads/openmpi-3.0.1.tar.gz

```
gunzip -c openmpi-3.0.1.tar.gz | tar xf -
cd openmpi-3.0.1
./configure
sudo make all install
```

Compilar con 

`mpicc -o <ejecutable> <codigo>`

Ejecutar con

`mpirun -np <nro procesos> <ejecutable>`

o si hay un hostfile

`mpirun --hostfile <hostfile> -np <nro procesos> <ejecutable>`