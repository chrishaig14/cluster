#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../matrix.h"

void row_col_mult(int*** row_matrix, int*** col_matrix, int** result, int m, int n, int r, int k) {
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < r; j++) {
            result[i][j] = 0;
        }
    }

    for (size_t i = 0; i < k; i++) {
        int** partial_result = malloc2d(m, r);
        print_matrix(row_matrix[i], m, n);
        print_matrix(col_matrix[i], n, r);
        matrix_mult(row_matrix[i], col_matrix[i], partial_result, m, n, r);
        print_matrix(partial_result, m, r);
        matrix_sum(result, partial_result, result, m, r);
        free2d(partial_result);
    }
}

MPI_Datatype create_submatrix_type(int M, int N, int m, int n) {
    // tamaño de la matriz global
    int size[2] = {M, N};
    // subsizes: tamaños de la submatriz local
    int subsizes[2] = {m, n};
    // starts: donde empieza las submatrices dentro de la matriz global
    int start[2] = {0, 0};

    MPI_Datatype type;
    MPI_Datatype new_type;

    MPI_Type_create_subarray(2, size, subsizes, start, MPI_ORDER_C, MPI_INT, &type);
    MPI_Type_create_resized(type, 0, n * sizeof(int), &new_type);  //???
    MPI_Type_commit(&new_type);
    MPI_Type_free(&type);
    return new_type;
}

int* calculate_displacements(int M, int N, int N_PROC) {

    int* displs = malloc(N_PROC * N_PROC * sizeof(int));
    for (size_t i = 0; i < N_PROC; i++) {
        for (size_t j = 0; j < N_PROC; j++) {
            displs[i * N_PROC + j] = i * M  + j;

        }
    }

    return displs;
}

int** scatter_matrix(int** global, int M, int N, int N_PROC, int* pm, int* pn) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    *pm = M / N_PROC;
    *pn = N / N_PROC;

    int m = *pm;
    int n = *pn;

    int** local = malloc2d(m, n);

    if (rank == 0) {
        int* globalptr = &(global[0][0]);

        int* send_counts = malloc(num_proc * sizeof(int));
        for (size_t i = 0; i < num_proc; i++) send_counts[i] = 1;

        int* displs = calculate_displacements(M, N, N_PROC);

        MPI_Datatype new_type = create_submatrix_type(M, N, m, n);

        MPI_Scatterv(globalptr, send_counts, displs, new_type, &(local[0][0]), m * n, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Type_free(&new_type);
        free(send_counts);
        free(displs);
    } else {
        MPI_Scatterv(NULL, NULL, NULL, NULL, &(local[0][0]), m * n, MPI_INT, 0, MPI_COMM_WORLD);
    }

    return local;
}

void gather_matrix(int** local, int m, int n, int M, int N, int N_PROC, int** global) {
    int num_proc;  // num_proc es un cuadrado: 4, 9, 16, 25, 36, etc...

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        int* globalptr = &(global[0][0]);

        int* send_counts = malloc(num_proc * sizeof(int));
        for (size_t i = 0; i < num_proc; i++) send_counts[i] = 1;

        int* displs = calculate_displacements(M, N, N_PROC);

        MPI_Datatype new_type = create_submatrix_type(M, N, m, n);

        MPI_Gatherv(&(local[0][0]), m * n, MPI_INT, globalptr, send_counts, displs, new_type, 0, MPI_COMM_WORLD);

        MPI_Type_free(&new_type);
        free(send_counts);
        free(displs);
    } else {
        MPI_Gatherv(&(local[0][0]), m * n, MPI_INT, NULL, NULL, NULL, NULL, 0, MPI_COMM_WORLD);
    }
}

int** complete_with_zeros(int** a, int m, int n, int p, int q) {
    int** copy = malloc2d(p, q);
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            copy[i][j] = a[i][j];
        }
    }

    for (size_t i = m; i < p; i++) {
        for (size_t j = n; j < q; j++) {
            copy[i][j] = 0;
        }
    }
    return copy;
}

int** remove_zeros(int** a, int p, int q, int m, int n) {
    int** copy = malloc2d(m, n);
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            copy[i][j] = a[i][j];
        }
    }

    return copy;
}

int main(int argc, char const* argv[]) {

    if (argc != 5) {
        printf("Usage: summa N_PROC matrix_a matrix_b result\n");
        return 0;
    }

    MPI_Init(NULL, NULL);

    int** global_a = NULL;
    int** global_b = NULL;
    int** global_result = NULL;

    // sqrt of number of processors
    int N_PROC = atoi(argv[1]);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Working on a grid of %i x %i processors\n", N_PROC, N_PROC);

    printf("RANK: %i\n", rank);

    int M, N, R, M_O, N_O, R_O;

    if (rank == 0) {
        int m_a, n_a, m_b, n_b;
        printf("Reading matrix A...\n");
        global_a = parse_matrix(argv[2], &m_a, &n_a);
        printf("A: %i x %i\n", m_a, n_a);
        printf("OK\n");
        printf("Reading matrix B...\n");
        global_b = parse_matrix(argv[3], &m_b, &n_b);
        printf("B: %i x %i\n", m_b, n_b);
        printf("OK\n");

        if (n_a != m_b) {
            fprintf(stderr, "error: Can't multiply matrices, wrong dimensions!\n");
            free2d(global_a);
            free2d(global_b);
            return -1;
        }

        M_O = m_a;
        N_O = n_a;
        R_O = n_b;

        int k = 0;
        while ((M_O + k) % N_PROC != 0) k++;
        M = M_O + k;
        k = 0;
        while ((N_O + k) % N_PROC != 0) k++;
        N = N_O + k;
        k = 0;
        while ((R_O + k) % N_PROC != 0) k++;
        R = R_O + k;

        int** new_a = complete_with_zeros(global_a, m_a, n_a, M, N);
        int** new_b = complete_with_zeros(global_b, m_b, n_b, N, R);
        free2d(global_a);
        free2d(global_b);
        global_a = new_a;
        global_b = new_b;

        global_result = malloc2d(M, R);

    }

    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&R, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int m_a, n_a, m_b, n_b;

    int** local_a = scatter_matrix(global_a, M, N, N_PROC, &m_a, &n_a);
    printf("Received A!\n");
    print_matrix(local_a, m_a, n_a);

    int** local_b = scatter_matrix(global_b, N, R, N_PROC, &m_b, &n_b);
    printf("Received B!\n");
    print_matrix(local_b, m_b, n_b);

    if (n_a != m_b) {
        fprintf(stderr, "ERROR!!!\n");
        return -1;
    }

    int m, n, r;

    m = m_a;
    n = n_a;
    r = n_b;

    // número de fila y de columna del proceso

    int my_row = rank / N_PROC;
    int my_col = rank % N_PROC;

    // crear los grupos para cada fila y cada columna

    MPI_Comm ROW_COMM;
    MPI_Comm COL_COMM;

    MPI_Comm_split(MPI_COMM_WORLD, my_row, my_col, &ROW_COMM);
    MPI_Comm_split(MPI_COMM_WORLD, my_col, my_row, &COL_COMM);

    int*** row_matrix = malloc(N_PROC * sizeof(int**));
    int*** col_matrix = malloc(N_PROC * sizeof(int**));

    for (size_t j = 0; j < N_PROC; j++) {
        row_matrix[j] = malloc2d(m, n);
        col_matrix[j] = malloc2d(n, r);
    }

    free2d(row_matrix[my_col]);
    free2d(col_matrix[my_row]);

    row_matrix[my_col] = local_a;
    col_matrix[my_row] = local_b;

    for (size_t j = 0; j < N_PROC; j++) {
        // broadcast row
        MPI_Bcast(&(row_matrix[j][0][0]), m * n, MPI_INT, j, ROW_COMM);
    }

    for (size_t i = 0; i < N_PROC; i++) {
        // broadcast col
        MPI_Bcast(&(col_matrix[i][0][0]), n * r, MPI_INT, i, COL_COMM);
    }

    int** result = malloc2d(m, r);
    row_col_mult(row_matrix, col_matrix, result, m, n, r, N_PROC);

    printf("Local result: \n");

    print_matrix(result, m, r);

    gather_matrix(result, m, r, M, R, N_PROC, global_result);

    if (rank == 0) {
        printf("Received result matrix:\n");
        print_matrix(global_result, M, R);
        int** result_ok = remove_zeros(global_result, M, R, M_O, R_O);
        free2d(global_result);
        write_matrix(result_ok, M_O, R_O, argv[4]);
        free2d(result_ok);
    }

    free2d(result);

    MPI_Comm_free(&ROW_COMM);
    MPI_Comm_free(&COL_COMM);

    MPI_Finalize();

    for (size_t i = 0; i < N_PROC; i++) {
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
