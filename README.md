# TP Final: Cluster MPI - Sistemas Distribuidos - FIUBA

El siguiente trabajo consiste en armar un cluster MPI. Para lo cual, el cluster cuenta con un nodo maestro, y nodos esclavos. El nodo maestro controla el cluster y ejecuta el programa. En los nodos esclavos se ejecuta la aplicación distribuida desde el nodo maestro.

## Configuración del cluster

### Instalación de OpenMPI

Se debe instalar OpenMPI en los nodos maestro y esclavo

- Descargar MPI: https://www.open-mpi.org/software/ompi/v3.0/downloads/openmpi-3.0.1.tar.gz
- `tar -xzvf openmpi-3.0.1.tar.gz`
- `cd openmpi-3.0.1`
- `./configure`
- `sudo make all install`
- `sudo apt-get install libopenmpi-dev`

Verificar que OpenMPI este instalado y sea la misma versión en todos los nodos.
- `mpirun --version`

### Instalación de paquetes necesarios en el nodo maestro

- `sudo apt-get install openssh-server`
- `sudo apt-get install nfs-kernel-server nfs-common portmap`

### Instalación de paquetes necesarios en los nodos esclavos

- `sudo apt-get install ssh` ¿Hace falta?
- `sudo apt-get install openssh-server`
- `sudo apt-get install nfs-common portmap`

## Configuración del archivo hosts

Todos los nodos, tanto maestro como esclavo, deberán agregar las direcciones IP, en el archivo hosts:

- `sudo nano /etc/hosts`

```
127.0.0.1	localhost
#127.0.1.1	localhost

192.168.0.100 master
192.168.0.101 slave1
192.168.0.102 slave2
192.168.0.103 slave3
```

## Configuración de SSH desde el nodo maestro

Desde el nodo maestro, se debe configurar el acceso a los nodos esclavos mediante ssh. Esto se lo realiza, con el objetivo de que el nodo maestro pueda ejecutar comandos en los nodos esclavos.
Ademas se configura el acceso mediante clave publica rsa, y así evitar que el nodo maestro tenga que ingresar mediante una clave a los nodos esclavos.

En cada nodo esclavo se deberán ejecutar los siguientes comandos:

- `mkdir .ssh`
- `chmod 700 .ssh/`

Para la configuración, desde el nodo maestro, se deben realizar los siguientes pasos:

Generar la clave publica:

- `ssh-keygen -t rsa`

Acceder al directorio y copiar el archivo generado con la clave a cada nodo esclavo:

- `cd $HOME/.ssh/`
- `scp id_rsa.pub userSlave@hostname:/home/userSlave/.ssh/id_rsa.pub`

Acceder al nodo esclavo desde el nodo maestro:

- `ssh hostname`

Agregar la clave publica, y darle los permisos correspondientes:

- `cat id_rsa.pub >> authorized_keys`
- `chmod go-rwx authorized_keys id_rsa.pub`
 
Salir del nodo esclavo, y agregar la clave

- `logout`
- ``` eval `ssh-agent` ```
- `ssh-add`
- `ssh-add id_rsa`

## Configuracion de directorio compartido

Desde el nodo maestro se debe crear un directorio y compartirlo con los nodos esclavos. En este directorio, el nodo maestro, ejecutara el programa sobre los nodos esclavos.

### Configurar el directorio compartido en el nodo maestro

Generar el directorio:

- `mkdir cloud`

Exportar el directorio generado, editando el archivo /etc/exports:

- `sudo nano /etc/exports`

Agregar al final del archivo la siguiente linea:

- `/home/userMaster/cloud *(rw,no_subtree_check,async,no_root_squash)`

Finalmente se reinicia el sistema nfs:

- `/etc/init.d/nfs-kernel-server restart`

### Configurar el directorio compartido en los nodos esclavos

Ver si el directorio, generado en el nodo maestro, es accesible:

- `showmount -e master`

Crear el directorio y montarlo:

- `mkdir cloud`
- `sudo mount -t nfs master:/home/userMaster/cloud ~/cloud`
- `mount`

Para que el directorio permanezca montado, luego de reiniciar el sistema, se deberá editar el archivo /etc/fstab:

- `sudo nano /etc/fstab`

Y agregar al final del archivo, la siguiente linea:
```
master:/home/userMaster/cloud /home/userSlave/cloud nfs
```

## Ejecutar un programa MPI

Desde el nodo maestro, se deben copiar los programas escritos con MPI en el directorio compartido, y compilarlos mediante el comando:

- `mpicc -o output program.c`

Se deberá crear un archivo hostfile, con la información de los nodos:

```
#Nodo maestro
master slots=2
#Nodos esclavo
slave1 slots=2
slave2 slots=2
slave3 slots=3
```

Donde los slots son la cantidad de procesadores de cada nodo.

Finalmente, el programa se ejecuta, desde el nodo maestro con el siguiente comando:

- `mpirun --hostfile <hostfile> -np <nro procesos> <ejecutable>`

## Prueba del cluster MPI

Para probar el cluster MPI, se desarrollaron programas que realicen productos entre dos matrices.

### Descomposición de matrices por filas

El programa consiste, en tomar la primer matriz, y dividir las filas entre los distintos procesadores. Después se toma la segunda matriz, y se divide las columnas entre los distintos procesadores.
Cada proceso calcula los productos entre las filas y columnas que recibió, calculando un resultado parcial de la matriz resultado. Después cada proceso enviá las columnas de la segunda matriz, a su siguiente vecino, determinado por el ranking de MPI. Y recibe las columnas de la segunda matriz de su vecino anterior. Estos pasos se repiten hasta que cada proceso calcula las filas correspondientes, de la matriz resultado.
Finalmente cada proceso enviá las filas calculadas al proceso maestro, obteniendo la matriz resultado.

### SUMMA

Este algoritmo divide el trabajo a realizar en una grilla de P x P procesos. 

Dadas las matrices a multiplicar A de M x N y B de N x R, cada matriz se subdivide en submatrices de M/P x N/P para A, y N/P x R/P para B. Cada proceso recibe desde el proceso root una submatriz de A y una de B (la que corresponde a su posición en la grilla de procesos). 

A continuación cada proceso hace un broadcast de las submatrices que recibió del root: envía la submatriz de A que recibió a todos los procesos en su misma fila; y la de B a todos los procesos en su misma columna. 

Cuando todos los procesos hayan hecho broadcast de sus submatrices, cada uno podrá de forma independiente a los demás calcular el producto correspondiente a una submatriz de M/P x R/P de la matriz resultado C, de M x R. 

Finalmente, todos los procesos envían su submatriz resultado al proceso root, que arma la matriz resultado C completa.

### Cannon


## Documentación:

- https://www.open-mpi.org/doc/v3.0/
- Tutorial de MPI: http://mpitutorial.com/tutorials/
- Scalable Universal Matrix Multiplication Algorithm (SUMMA): http://www.netlib.org/lapack/lawnspdf/lawn96.pdf