#include "matrix.h"
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