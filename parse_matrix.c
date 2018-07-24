#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int** parse_matrix_txt(const char* filename, int* m, int* n) {
    FILE* file = fopen(filename, "r");
    int num;
    while (fscanf(file, "%i", &num) != EOF) {
    }
}

int** parse_matrix(const char* filename, int* pm, int* pn) {
    FILE* file = fopen(filename, "rb");
    int num;

    fread(pm, sizeof(int), 1, file);
    fread(pn, sizeof(int), 1, file);
    int m = *pm;
    int n = *pn;
    printf("%i X %i\n", m, n);
    getchar();
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

int main(int argc, char const* argv[]) {
    int m;
    int n;
    int** a = parse_matrix(argv[1], &m, &n);
    free2d(a);
}
