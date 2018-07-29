int** malloc2d(int m, int n);

void free2d(int** a);

void matrix_sum(int** a, int** b, int** c, int m, int n);

void matrix_mult(int** a, int** b, int** c, int m, int n, int r);

void write_matrix(int** a, int m, int n, const char* filename);

int** parse_matrix(const char* filename, int* pm, int* pn);

void print_matrix(int** a, int m, int n);
