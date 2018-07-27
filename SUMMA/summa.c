#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../matrix.h"

#define N 6

int** matrix_mult(int** a, int** b, int** c, int m, int n, int r) {
    // a = m x n, b = n x r, c = m x r

    uint64_t count = 0;

    int total = m * r;

    int prev_perc = -1;
    int perc = 0;
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < r; j++) {
            c[i][j] = 0;
            for (size_t k = 0; k < n; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
            count++;
            perc = (float)count / total * 100;
            if (perc != prev_perc) {
                printf("%i %%\n", perc);
                prev_perc = perc;
            }
        }
    }
    return c;
}

MPI_Datatype create_submatrix_type(int bs) {
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
    MPI_Type_free(&type);
    return new_type;
}

int* calculate_displacements(int num_proc, int s_num_proc, int bs) {
    int* displs = malloc(num_proc * sizeof(int));
    for (size_t i = 0; i < s_num_proc; i++) {
        for (size_t j = 0; j < s_num_proc; j++) {
            displs[i * s_num_proc + j] = i * s_num_proc * bs + j % s_num_proc;
            printf("displ %i:  %i\n", i * s_num_proc + j, displs[i * s_num_proc + j]);
        }
    }
    return displs;
}

int** scatter_matrix(int** global, int gm, int gn, int* m, int* n) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int s_num_proc = sqrt(num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int bs = N / s_num_proc;

    int** local = malloc2d(bs, bs);

    *m = bs;
    *n = bs;

    if (rank == 0) {
        int* globalptr = &(global[0][0]);

        int* send_counts = malloc(num_proc * sizeof(int));
        for (size_t i = 0; i < num_proc; i++) send_counts[i] = 1;

        int* displs = calculate_displacements(num_proc, s_num_proc, bs);

        MPI_Datatype new_type = create_submatrix_type(bs);

        MPI_Scatterv(globalptr, send_counts, displs, new_type, &(local[0][0]), bs * bs, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Type_free(&new_type);
        free(send_counts);
        free(displs);
    } else {
        MPI_Scatterv(NULL, NULL, NULL, NULL, &(local[0][0]), bs * bs, MPI_INT, 0, MPI_COMM_WORLD);
    }

    return local;
}

void gather_matrix(int** local, int m, int n, int** global) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int s_num_proc = sqrt(num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int bs = N / s_num_proc;

    if (rank == 0) {
        int* globalptr = &(global[0][0]);

        int* send_counts = malloc(num_proc * sizeof(int));
        for (size_t i = 0; i < num_proc; i++) send_counts[i] = 1;

        int* displs = calculate_displacements(num_proc, s_num_proc, bs);

        MPI_Datatype new_type = create_submatrix_type(bs);

        MPI_Gatherv(&(local[0][0]), m * n, MPI_INT, globalptr, send_counts, displs, new_type, 0, MPI_COMM_WORLD);

        MPI_Type_free(&new_type);
        free(send_counts);
        free(displs);
    } else {
        MPI_Gatherv(&(local[0][0]), m * n, MPI_INT, NULL, NULL, NULL, NULL, 0, MPI_COMM_WORLD);
    }
}

void print_matrix(int** a, int m, int n) {
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            printf("%i\t", a[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char const* argv[]) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Init(NULL, NULL);

    int** global = NULL;

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        global = malloc2d(N, N);
        int n = 0;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                global[i][j] = n;
                n++;
            }
        }

        print_matrix(global, N, N);
    }

    int m, n;
    printf("RANK: %i\n", rank);
    printf("Waiting to receive matrix...\n");
    int** local = scatter_matrix(global, N, N, &m, &n);
    printf("Received!\n");
    print_matrix(local, m, n);

    // Acá se hace el cálculo

    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            local[i][j] *= local[i][j];
        }
    }

    printf("Partial result\n");

    print_matrix(local, m, n);

    // int** result = malloc2d(m,m);

    // matrix_mult(local, local, result, m, m, m);

    gather_matrix(local, m, n, global);

    if (rank == 0) {
        printf("Received result matrix:\n");
        print_matrix(global, N, N);
        free2d(global);
    }
    free2d(local);

    MPI_Finalize();
    return 0;
}
