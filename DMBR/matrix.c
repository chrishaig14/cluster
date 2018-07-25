#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "const.h"
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


void printMatrix(int* matrix,int r,int c) {

	
	for (size_t i = 0; i < r; i++) {
		for (size_t j = 0; j < c; j++) {
			printf("%d ", matrix[j+i*c]);
		}
		printf("\n");
	}

	printf("\n");

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