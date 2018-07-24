#include "matrix.h"
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