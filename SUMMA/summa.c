#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../matrix.h"

void matrix_sum(int** a, int** b, int** c, int m, int n) {
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
}

void matrix_mult(int** a, int** b, int** c, int m, int n, int r) {
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
}

void row_col_mult(int*** row_matrix, int*** col_matrix, int** result, int m, int n, int r, int k) {
    printf("Filling result with zeroes...\n");
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < r; j++) {
            result[i][j] = 0;
        }
    }

    print_matrix(result, m, r);

    printf("OK\n");

    for (size_t i = 0; i < k; i++) {
        int** partial_result = malloc2d(m, r);
        printf("Partial multiplication...\n");
        printf("First matrix: \n");
        print_matrix(row_matrix[i], m, n);
        printf("Second matrix: \n");
        print_matrix(col_matrix[i], n, r);
        matrix_mult(row_matrix[i], col_matrix[i], partial_result, m, n, r);
        printf("Resulting matrix: \n");
        print_matrix(partial_result, m, r);
        printf("Ready\n");
        matrix_sum(result, partial_result, result, m, r);
        free2d(partial_result);
    }
}

MPI_Datatype create_submatrix_type(int M, int N, int m, int n) {
    printf("M = %i\n", M);
    printf("N = %i\n", N);
    printf("m = %i\n", m);
    printf("n = %i\n", n);
    // tamaño de la matriz global
    int size[2] = {M, N};
    // subsizes: tamaños de la submatriz local
    int subsizes[2] = {m, n};
    // starts: donde empieza las submatrices dentro de la matriz global
    int start[2] = {0, 0};

    MPI_Datatype type;
    MPI_Datatype new_type;

    MPI_Type_create_subarray(2, size, subsizes, start, MPI_ORDER_C, MPI_INT, &type);
    printf("OK1\n");
    MPI_Type_create_resized(type, 0, m * sizeof(int), &new_type);
    printf("OK2\n");
    MPI_Type_commit(&new_type);
    printf("OK3\n");
    MPI_Type_free(&type);
    printf("OK4\n");
    return new_type;
}

int* calculate_displacements(int M, int N, int m_proc, int n_proc) {
    printf("M = %i\n", M);
    printf("N = %i\n", N);
    printf("m_proc = %i\n", m_proc);
    printf("n_proc = %i\n", n_proc);
    int* displs = malloc(m_proc * n_proc * sizeof(int));
    for (size_t i = 0; i < m_proc; i++) {
        for (size_t j = 0; j < n_proc; j++) {
            displs[i * n_proc + j] = i * (M/m_proc) * n_proc + j;
            printf("displ %i:  %i\n", i * n_proc + j, displs[i * n_proc + j]);
        }
    }
    getchar();
    return displs;
}

int** scatter_matrix(int** global, int M, int N, int m_proc, int n_proc, int* m, int* n) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int** local = malloc2d(3, 3);

    *m = 3;
    *n = 3;

    if (rank == 0) {
        int* globalptr = &(global[0][0]);

        int* send_counts = malloc(num_proc * sizeof(int));
        for (size_t i = 0; i < num_proc; i++) send_counts[i] = 1;

        int* displs = calculate_displacements(M, N, m_proc, n_proc);

        MPI_Datatype new_type = create_submatrix_type(M, N, *m, *n);

        MPI_Scatterv(globalptr, send_counts, displs, new_type, &(local[0][0]), (*m) * (*n), MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Type_free(&new_type);
        free(send_counts);
        free(displs);
    } else {
        MPI_Scatterv(NULL, NULL, NULL, NULL, &(local[0][0]), (*m) * (*n), MPI_INT, 0, MPI_COMM_WORLD);
    }

    return local;
}

void gather_matrix(int** local, int m, int n, int M, int N, int m_proc, int n_proc, int** global) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        int* globalptr = &(global[0][0]);

        int* send_counts = malloc(num_proc * sizeof(int));
        for (size_t i = 0; i < num_proc; i++) send_counts[i] = 1;

        int* displs = calculate_displacements(M, N, m_proc, n_proc);

        MPI_Datatype new_type = create_submatrix_type(M, N, m, n);

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

    // for(size_t i = 0; i < argc; i++){
    //     printf("argv[%i] = %s\n", i, argv[i]);
    // }

    if (argc != 6) {
        printf("Usage: summa m_proc n_proc matrix_a matrix_b result\n");
        return 0;
    }

    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Init(NULL, NULL);

    int** global_a = NULL;
    int** global_b = NULL;
    int** global_result = NULL;

    // number of processor rows
    int m_proc = atoi(argv[1]);
    // number of processor cols
    int n_proc = atoi(argv[2]);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Working on a grid of %i x %i processors\n", m_proc, n_proc);

    printf("RANK: %i\n", rank);

    int M, N, R;

    if (rank == 0) {
        int m_a, n_a, m_b, n_b;
        printf("Reading matrix A...\n");
        global_a = parse_matrix(argv[3], &m_a, &n_a);
        printf("A: %i x %i\n", m_a, n_a);
        printf("OK\n");
        printf("Reading matrix B...\n");
        global_b = parse_matrix(argv[4], &m_b, &n_b);
        printf("B: %i x %i\n", m_b, n_b);
        printf("OK\n");

        if (n_a != m_b) {
            fprintf(stderr, "error: Can't multiply matrices, wrong dimensions!\n");
            free2d(global_a);
            free2d(global_b);
            return -1;
        }
        M = m_a;
        N = n_a;
        R = n_b;
        global_result = malloc2d(M, R);
        printf("A: %i x %i\n B: %i x %i\n", M, N, N, R);
        printf("ENTER to start working...");
        getchar();

        // global = malloc2d(N, N);
        // int n = 0;
        // for (size_t i = 0; i < N; i++) {
        //     for (size_t j = 0; j < N; j++) {
        //         global[i][j] = rand() % 5;
        //         n++;
        //     }
        // }

        // print_matrix(global, N, N);
    }

    int m, n;
    printf("Waiting to receive matrix...\n");
    int** local_a = scatter_matrix(global_a, M, N, m_proc, n_proc, &m, &n);
    printf("Received A!\n");
    print_matrix(local_a, m, n);
    printf("ENTER to continue...");
    getchar();
    int** local_b = scatter_matrix(global_b, M, N, m_proc, n_proc, &m, &n);
    printf("Received B!\n");
    print_matrix(local_b, m, n);
    printf("ENTER to continue...");
    getchar();
    

    // Acá se hace el cálculo

    // for (size_t i = 0; i < m; i++) {
    //     for (size_t j = 0; j < n; j++) {
    //         local[i][j] *= local[i][j];
    //     }
    // }

    // printf("Partial result\n");

    // print_matrix(local, m, n);

    // int** result = malloc2d(m,m);

    // matrix_mult(local, local, result, m, m, m);

    // número de fila y de columna del proceso

    int my_row = rank / 2;
    int my_col = rank % 2;

    // crear los grupos para cada fila y cada columna

    MPI_Comm ROW_COMM;
    MPI_Comm COL_COMM;

    MPI_Comm_split(MPI_COMM_WORLD, my_row, my_col, &ROW_COMM);
    MPI_Comm_split(MPI_COMM_WORLD, my_col, my_row, &COL_COMM);

    int*** row_matrix = malloc(2 * sizeof(int**));
    int*** col_matrix = malloc(2 * sizeof(int**));

    printf("Malloc-ing...\n");

    for (size_t j = 0; j < 2; j++) {
        row_matrix[j] = malloc2d(m, n);
    }

    for (size_t i = 0; i < 2; i++) {
        col_matrix[i] = malloc2d(m, n);
    }

    free2d(row_matrix[my_col]);
    free2d(col_matrix[my_row]);

    row_matrix[my_col] = local_a;
    col_matrix[my_row] = local_b;

    printf("OK\n");

    for (size_t j = 0; j < 2; j++) {
        // broadcast row
        if (j == my_col) {
            printf("Broadcasting...\n");
            printf("\n");
            print_matrix(row_matrix[j], m, n);
            printf("\n");
        }

        MPI_Bcast(&(row_matrix[j][0][0]), m * n, MPI_INT, j, ROW_COMM);
    }

    for (size_t i = 0; i < 2; i++) {
        // broadcast col
        MPI_Bcast(&(col_matrix[i][0][0]), m * n, MPI_INT, i, COL_COMM);
    }

    printf("Current row: \n");

    for (size_t j = 0; j < 2; j++) {
        printf("\n");
        print_matrix(row_matrix[j], m, n);
        printf("\n");
    }

    printf("Current column: \n");

    for (size_t i = 0; i < 2; i++) {
        printf("\n");
        print_matrix(col_matrix[i], m, n);
        printf("\n");
    }

    printf("Calculating multiplication...\n");

    int** result = malloc2d(3, 3);
    row_col_mult(row_matrix, col_matrix, result, 3, 3, 3, 2);

    printf("Local result: \n");

    print_matrix(result, 3, 3);

    gather_matrix(result, m, n, M, N, m_proc, n_proc, global_result);

    if (rank == 0) {
        printf("Received result matrix:\n");
        print_matrix(global_result, M, R);
        free2d(global_result);
    }

    free2d(result);

    MPI_Comm_free(&ROW_COMM);
    MPI_Comm_free(&COL_COMM);

    MPI_Finalize();

    for (size_t i = 0; i < 2; i++) {
        if (i == my_col) {
            free2d(col_matrix[i]);
        } else if (i != my_row) {
            free2d(row_matrix[i]);
            free2d(col_matrix[i]);
        }
    }
    free(row_matrix);
    free(col_matrix);

    return 0;
}
