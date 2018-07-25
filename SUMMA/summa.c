#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../matrix.h"
#define N 6

int main(int argc, char const* argv[]) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int s_num_proc = sqrt(num_proc);
    printf("S_NUM_PROC %i\n", s_num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // int** global = NULL;
    int* globalptr = NULL;
    int** global = NULL;

    int bs = N / s_num_proc;
    printf("BS %i\n", bs);

    // al 0 le llega 1 int; al 1, 2; al 2, 3; al 3, 10; empezando en las posiciones en displs
    // size: tamaño de la matriz
    int size[2] = {N, N};
    // subsizes: tamaños de la submatriz
    int subsizes[2] = {bs, bs};
    // starts: donde empieza la submatriz
    int start[2] = {0, 0};

    MPI_Datatype type;
    MPI_Datatype new_type;

    MPI_Type_create_subarray(2, size, subsizes, start, MPI_ORDER_C, MPI_INT, &type);
    MPI_Type_create_resized(type, 0, bs * sizeof(int), &new_type);
    MPI_Type_commit(&new_type);

    if (rank == 0) {
        global = malloc2d(N, N);
        globalptr = &(global[0][0]);
        int n = 0;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                global[i][j] = n;
                n++;
            }
        }

        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                printf("%i\t", global[i][j]);
            }
            printf("\n");
        }
    }

    int* send_counts = malloc(num_proc * sizeof(int));
    // cada proceso recibe una submatriz

    for (size_t i = 0; i < num_proc; i++) {
        send_counts[i] = 1;
    }

    int* displs = malloc(num_proc * sizeof(int));
    for (size_t i = 0; i < s_num_proc; i++) {
        for (size_t j = 0; j < s_num_proc; j++) {
            displs[i * s_num_proc + j] = i * s_num_proc * bs + j % s_num_proc;
            printf("displ %i:  %i\n", i * s_num_proc + j, displs[i * s_num_proc + j]);
        }
    }

    int** local = malloc2d(bs, bs);

    for (size_t i = 0; i < num_proc; i++) {
        printf("send_counts[%i] = %i\n", i, send_counts[i]);
    }

    for (size_t i = 0; i < num_proc; i++) {
        printf("displ[%i] = %i\n", i, displs[i]);
    }

    MPI_Scatterv(globalptr, send_counts, displs, new_type, &(local[0][0]), bs * bs, MPI_INT, 0, MPI_COMM_WORLD);

    printf("RANK: %i\n", rank);
    printf("Waiting to receive matrix...\n");


    printf("Received!\n");

    for (size_t i = 0; i < bs; i++) {
        for (size_t j = 0; j < bs; j++) {
            printf("%i\t", local[i][j]);
        }
        printf("\n");
    }

    if (rank == 0) {
        free2d(global);
    }
    free2d(local);
    free(send_counts);
    free(displs);
    MPI_Type_free(&type);
    MPI_Type_free(&new_type);
    MPI_Finalize();
    return 0;
}
