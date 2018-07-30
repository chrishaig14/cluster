#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "matrix.h"

int main(int argc, char const* argv[]) {
    if (argc != 5) {
        printf("Usage: random_matrix m n output val_max");
        return 0;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    char const* output = argv[3];
    int val_max = atoi(argv[4]);

    if (val_max < 0) {
        fprintf(stderr, "val_max can't be negative or 0!\n");
        return -1;
    }

    srand(time(NULL));

    int** a = malloc2d(m, n);

    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            a[i][j] = rand() % val_max;
        }
    }

    print_matrix(a, m, n);

    write_matrix(a, m, n, output);

    free2d(a);

    return 0;
}
