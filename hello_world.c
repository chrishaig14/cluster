#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    // Inicializar MPI
    MPI_Init(NULL, NULL);

    // Obtener el numero de procesos
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Obtener el rank (id) del proceso actual
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Obtener el nombre del procesador actual
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf(
        "Hello world from processor %s, rank %d"
        " out of %d processes\n",
        processor_name, world_rank, world_size);

    // Finalizar MPI
    MPI_Finalize();
}