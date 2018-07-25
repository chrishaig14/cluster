#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 0
#define MATRIX_ROW 1

int** create_empty_matrix(int n) {
    int** a = (int**)malloc(n * sizeof(int*));

    for (size_t i = 0; i < n; i++) {
        a[i] = (int*)malloc(n * sizeof(int));
    }

    return a;
}

int** create_random_matrix(int n) {
    int** a = create_empty_matrix(n);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            a[i][j] = rand() % 1000000;
        }
    }

    return a;
}

void delete_matrix(int** a, int n) {
    for (size_t i = 0; i < n; i++) {
        free(a[i]);
    }
    free(a);
}

void send_matrix(int** a, int n, int dest) {
    MPI_Send(&n, 1, MPI_INT, dest, MATRIX_SIZE, MPI_COMM_WORLD);

    for (size_t i = 0; i < n; i++) {
        MPI_Send(a[i], n, MPI_INT, dest, MATRIX_ROW, MPI_COMM_WORLD);
    }
}

int** receive_matrix(int* n) {
    MPI_Recv(n, 1, MPI_INT, 0, MATRIX_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int** a = create_empty_matrix(*n);
    for (size_t i = 0; i < (*n); i++) {
        MPI_Recv(a[i], (*n), MPI_INT, 0, MATRIX_ROW, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }

    return a;
}

int** matrix_mult_partial(int** a, int** b, int n, int start_row, int end_row) {
    int** c = create_empty_matrix(n);
    uint64_t l = 0;

    for (size_t i = start_row; i <= end_row; i++) {
        for (size_t j = 0; j < n; j++) {
            c[i][j] = 0;
            for (size_t k = 0; k < n; k++) {
                l++;
                c[i][j] += a[i][k] * b[k][j];
                if (l % 1000000 == 0) {
                    printf("%f %% \n", (double)l / ((uint64_t)n * n * n) * 100);
                }
            }
        }
    }
    printf("L: %lu\n", l);
    return c;
}

int** matrix_mult(int** a, int** b, int n) {
    int** c = create_empty_matrix(n);
    uint64_t l = 0;

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            c[i][j] = 0;
            for (size_t k = 0; k < n; k++) {
                l++;
                c[i][j] += a[i][k] * b[k][j];
                if (l % 1000000 == 0) {
                    printf("%f %% \n", (double)l / ((uint64_t)n * n * n) * 100);
                }
            }
        }
    }
    printf("L: %lu\n", l);
    return c;
}

int main(int argc, char const* argv[]) {
    srand(time(NULL));

    MPI_Init(NULL, NULL);

    int num_proc;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int proc_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    if (proc_rank == 0) {
        int n = rand() % 4999 + 1;
        printf("MATRIX SIZE = %i\n", n);
        int** a = create_random_matrix(n);
        int** b = create_random_matrix(n);
        printf("Multiplying...\n");
        int** c = matrix_mult(a, b, n);
        printf("Ready!\n");

        for (size_t i = 1; i < num_proc; i++) {
            send_matrix(a, n, i);
            send_matrix(b, n, i);
        }

        delete_matrix(a, n);
        delete_matrix(b, n);

    } else {
        int** a = NULL;
        int** b = NULL;
        int n;
        a = receive_matrix(&n);
        printf("Received matrix A successfully %i, size %i\n", proc_rank, n);
        b = receive_matrix(&n);
        printf("Received matrix B successfully %i, size %i\n", proc_rank, n);
        delete_matrix(a, n);
        delete_matrix(b, n);
    }

    MPI_Finalize();

    return 0;
}