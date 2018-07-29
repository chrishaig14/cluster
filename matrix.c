#include "matrix.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int** malloc2d(int m, int n) {
    int** a = (int**)malloc(m * sizeof(int*));
    int* p = (int*)malloc(m * n * sizeof(int));

    for (size_t i = 0; i < m; i++) {
        a[i] = p + i * n;
    }

    return a;
}

void free2d(int** a) {
    free(a[0]);
    free(a);
}

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

void print_matrix(int** a, int m, int n) {
    if (m > 10 || n > 10) {
        printf("MATRIX TOO LARGE TO BE SHOWN!\n");
        return;
    }
    
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            printf("%i ", a[i][j]);
        }
        printf("\n");
    }
}

int** parse_matrix(const char* filename, int* pm, int* pn) {
    FILE* file = fopen(filename, "rb");
    int num;

    fread(pm, sizeof(int), 1, file);
    fread(pn, sizeof(int), 1, file);
    int m = *pm;
    int n = *pn;
    int** a = malloc2d(m, n);

    int i = 0;
    int j = 0;
    int count = 0;
    while (fread(&num, sizeof(num), 1, file) == 1) {
        i = count / n;
        j = count % n;

        a[i][j] = num;

        printf("%i\t", a[i][j]);
        if (j == n - 1) {
            printf("\n");
        }
        count++;
    };
    fclose(file);
    return a;
}

void write_matrix(int** a, int m, int n, const char* filename) {
    FILE* file = fopen(filename, "wb");

    fwrite(&m, sizeof(m), 1, file);
    fwrite(&n, sizeof(n), 1, file);

    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            fwrite(&(a[i][j]), sizeof(a[i][j]), 1, file);
        }
    }

    fclose(file);
}