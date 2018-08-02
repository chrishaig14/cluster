#include "matrix.h"

int main(int argc, char const* argv[]) {
    int m, n;
    int** a = parse_matrix(argv[1], &m, &n);
    print_matrix(a, m, n);
    free2d(a);
    return 0;
}
