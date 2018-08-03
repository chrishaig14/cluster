#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"



void validate_create_matrix(int* matrix){
	if (matrix == NULL) {
		perror("Error in malloc");
		exit(1);
	}
}

int* create_empty_matrix(int nrows,int ncols) {
	
	int *matrix = (int*) malloc(nrows*ncols*sizeof(int));
	validate_create_matrix(matrix);

	return matrix;
}


int* create_random_matrix(int nrows,int ncols,int extra_rows) {
	int* matrix = create_empty_matrix(nrows+extra_rows,ncols);
	int i;
	for (i=0; i<nrows*ncols; i++) {
		matrix[i] = rand() % 10;
	}
	for (i=nrows*ncols; i<(nrows+extra_rows)*ncols; i++) {
		matrix[i] = 0;
	}

	return matrix;
}

int* transpose_matrix(int* matrix, int nrows, int ncols,int extra_cols){
	int* transpose = create_empty_matrix(nrows,ncols+extra_cols);

	for (size_t i = 0; i < ncols; i++) {
		for (size_t j = 0; j < nrows; j++) {
			transpose[j+i*nrows] = matrix[i+j*ncols];
		}
	}
	for (size_t i=nrows*ncols; i<nrows*(ncols+extra_cols); i++) {
		transpose[i] = 0;
	}


	return transpose;
}

void delete_matrix(int* matrix) {
	free(matrix);
}

int* calculate_matrix_C(int* matrix,int rows,int cols,int cols_extra) {
	int* matrix_C = (int*) malloc(sizeof(int) * rows * cols);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			matrix_C[col+row*cols] = matrix[col+row*(cols+cols_extra)];
		}
	}
	return matrix_C;
}

void read_rows_cols(const char* filename, int* rows, int* cols) {

	FILE* file = fopen(filename, "rb");
	fread(rows, sizeof(int), 1, file);
    fread(cols, sizeof(int), 1, file);
    fclose(file);

}

int* parse_matrix(const char* filename, int extra_rows) {
    
    FILE* file = fopen(filename, "rb");
    int num;

    int rows,cols;

    fread(&rows, sizeof(int), 1, file);
    fread(&cols, sizeof(int), 1, file);

    int* matrix = (int*) malloc((rows+extra_rows)*cols*sizeof(int));

    int count = 0;
    while (fread(&num, sizeof(num), 1, file) == 1) {

        matrix[count] = num;
        count++;
    };

    for (count=rows*cols; count<(rows+extra_rows)*cols; count++) {
		matrix[count] = 0;
	}

    fclose(file);

	return matrix;

}

void write_matrix(int* a, int m, int n, const char* filename) {
    FILE* file = fopen(filename, "wb");

    fwrite(&m, sizeof(m), 1, file);
    fwrite(&n, sizeof(n), 1, file);

	for (size_t i = 0; i < m; i++) {
		for (size_t j = 0; j < n; j++) {
			fwrite(&(a[j+i*n]), sizeof(int), 1, file);
		}

	}


    fclose(file);
}