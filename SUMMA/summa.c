#include <mpi.h>
#include <stdio.h>
#include "../matrix.h"
#define N_ITEMS 16

int main(int argc, char const* argv[]) {
    int num_proc;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int** global;

    int local[2][2];

    // al 0 le llega 1 int; al 1, 2; al 2, 3; al 3, 10; empezando en las posiciones en displs
    // size: tamaño de la matriz
    int size[2] = {4, 4};
    // subsizes: tamaños de la submatriz
    int subsizes[2] = {2, 2};
    // starts: donde empieza la submatriz
    int start[2] = {0, 0};

    MPI_Datatype new_type;

    MPI_Type_create_subarray(2, size, subsizes, start, MPI_ORDER_C, MPI_INT, &new_type);

    MPI_Type_commit(&new_type);

    if (rank == 0) {
        global = malloc2d(4, 4);
        int n = 0;
        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 4; j++) {
                global[i][j] = n;
                n++;
            }
        }
        // esto funciona pero no sé si debería: mandarse un mensaje a sí mismo...
        MPI_Send(&(global[0][0]), 1, new_type, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&(global[0][2]), 1, new_type, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&(global[2][0]), 1, new_type, 2, 0, MPI_COMM_WORLD);
        MPI_Send(&(global[2][2]), 1, new_type, 3, 0, MPI_COMM_WORLD);        
    }
    printf("RANK: %i\n", rank);
    printf("Waiting to receive matrix...\n");
    MPI_Recv(local, 2 * 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Received!\n");

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            printf("%i\t", local[i][j]);
        }
        printf("\n");
    }

    if (rank == 0) {
        free2d(global);
    }

    MPI_Finalize();
    return 0;
}
