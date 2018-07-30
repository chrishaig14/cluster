#include <stdio.h>
#include "matrix.h"

int main(int argc, char const* argv[]) {
    if (argc != 4) {
        printf("Usage: mult matrix_a matrix_b result");
        return 0;
    }
    int m_a, n_a, m_b, n_b;
    int** a = parse_matrix(argv[1], &m_a, &n_a);
    int** b = parse_matrix(argv[2], &m_b, &n_b);

    if (n_a != m_b) {
        fprintf(stderr, "error: Wrong matrix dimensions!\n");
        return -1;
    }

    int m, n, r;

    m = m_a;
    n = n_a;
    r = n_b;

    int** c = malloc2d(m, r);

    matrix_mult(a, b, c, m, n, r);

    write_matrix(c, m, r, argv[3]);

    free2d(c);

    return 0;
}
